/* This file is part of RTags (http://rtags.net).

   RTags is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   RTags is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with RTags.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef FileMap_h
#define FileMap_h

#include <assert.h>
#include <functional>
#include <limits>

#ifndef _WIN32
#  include <sys/file.h>
#endif

#include "Location.h"
#include "rct/Serializer.h"
#include "rct/MemoryMappedFile.h"
#include "rct/WindowsUnicodeConversion.h"

template <typename T> inline static int compare(const T &l, const T &r)
{
    if (l < r)
        return -1;
    if (l > r)
        return 1;
    return 0;
}

template <> inline int compare(const String &l, const String &r)
{
    return l.compare(r);
}

template <> inline int compare(const Location &l, const Location &r)
{
    return l.compare(r);
}

template <typename Key, typename Value>
class FileMap
{
public:
    FileMap()
        : mCount(0), mValuesOffset(0), mOptions(0)
    {}

    void init()
    {
        memcpy(&mCount, mFile.filePtr(), sizeof(uint32_t));
        memcpy(&mValuesOffset, mFile.filePtr<char>() + sizeof(uint32_t), sizeof(uint32_t));
    }

    enum Options {
        None = 0x0,
        NoLock = 0x1
    };
    bool load(const Path &path, uint32_t options, String *error=0)
    {
        (void) error;
        typedef MemoryMappedFile MMF; // shorter name
        const MMF::LockType lck = !(options & NoLock) ? MMF::DO_LOCK : MMF::DONT_LOCK;

        if(!mFile.open(path, MemoryMappedFile::READ_ONLY, lck))
        {
            if(error) *error = "Could not map file";
            return false;
        }

        mOptions = options;
        init();
        return true;
    }

    Value value(const Key &key, bool *matched = 0) const
    {
        bool match;
        const uint32_t idx = lowerBound(key, &match);
        // error() << "value" << idx << key << match;
        if (matched)
            *matched = match;
        if (match)
            return valueAt(idx);
        return Value();
    }

    uint32_t count() const { return mCount; }

    Key keyAt(uint32_t index) const
    {
        assert(index >= 0 && index < mCount);
        return read<Key>(keysSegment(), index);
    }

    Value valueAt(uint32_t index) const
    {
        assert(index >= 0 && index < mCount);
        return read<Value>(valuesSegment(), index);
    }

    uint32_t lowerBound(const Key &k, bool *match = 0) const
    {
        if (!mCount) {
            if (match)
                *match = false;
            return std::numeric_limits<uint32_t>::max();

        }
        int lower = 0;
        int upper = mCount - 1;

        do {
            const int mid = lower + ((upper - lower) / 2);
            const int cmp = compare<Key>(k, keyAt(mid));
            if (cmp < 0) {
                upper = mid - 1;
            } else if (cmp > 0) {
                lower = mid + 1;
            } else {
                if (match)
                    *match = true;
                return mid;
            }
        } while (lower <= upper);

        if (lower == static_cast<int>(mCount))
            lower = std::numeric_limits<uint32_t>::max();
        if (match)
            *match = false;
        return lower;
    }

    static String encode(const Map<Key, Value> &map)
    {
        String out;
        Serializer serializer(out);
        serializer << static_cast<uint32_t>(map.size());
        uint32_t valuesOffset;
        if (uint32_t size = FixedSize<Key>::value) {
            valuesOffset = ((static_cast<uint32_t>(map.size()) * size) + (sizeof(uint32_t) * 2));
            serializer << valuesOffset;
            for (const std::pair<Key, Value> &pair : map) {
                out.append(reinterpret_cast<const char*>(&pair.first), size);
            }
        } else {
            serializer << static_cast<uint32_t>(0); // values offset
            uint32_t offset = sizeof(uint32_t) * 2 + (map.size() * sizeof(uint32_t));
            String keyData;
            Serializer keySerializer(keyData);
            for (const std::pair<Key, Value> &pair : map) {
                const uint32_t pos = offset + keyData.size();
                out.append(reinterpret_cast<const char*>(&pos), sizeof(pos));
                keySerializer << pair.first;
            }
            out.append(keyData);
            valuesOffset = out.size();
            memcpy(out.data() + sizeof(uint32_t), &valuesOffset, sizeof(valuesOffset));
        }
        assert(valuesOffset == static_cast<uint32_t>(out.size()));

        if (uint32_t size = FixedSize<Value>::value) {
            for (const std::pair<Key, Value> &pair : map) {
                out.append(reinterpret_cast<const char*>(&pair.second), size);
            }
        } else {
            const uint32_t encodedValuesOffset = valuesOffset + (sizeof(uint32_t) * map.size());
            String valueData;
            Serializer valueSerializer(valueData);
            for (const std::pair<Key, Value> &pair : map) {
                const uint32_t pos = encodedValuesOffset + valueData.size();
                out.append(reinterpret_cast<const char*>(&pos), sizeof(pos));
                valueSerializer << pair.second;
            }
            out.append(valueData);

        }
        return out;
    }

    /**
     * Write the FileMap to the specified file, overwriting all of its content.
     */
    static size_t write(const Path &path, const Map<Key, Value> &map, uint32_t options)
    {
#ifdef _WIN32
        const DWORD share = (!(options & NoLock)) ?
            0 : (FILE_SHARE_READ | FILE_SHARE_WRITE);
        HANDLE fd = CreateFileW(Utf8To16(path.nullTerminated()),
                                (GENERIC_WRITE), share, NULL,

                                // If the file does not exist, it is created.
                                // If the file already exists, it is truncated.
                                CREATE_ALWAYS,

                                FILE_ATTRIBUTE_NORMAL, NULL);

        if(fd == INVALID_HANDLE_VALUE)
        {
            // could not open file. Maybe the parent dir does not exist?
            if (!Path::mkdir(path.parentDir(), Path::Recursive)) {
                // couldn't create parent dir either :(
                return 0;
            }

            // Parent dir creation was successful. Try to open again...
            fd = CreateFileW(Utf8To16(path.nullTerminated()), GENERIC_WRITE, share,
                             NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if(fd == INVALID_HANDLE_VALUE)
            {
                // still no luck. Might also be because the file is locked
                // by someone else.
                return 0;
            }
        }

        // We don't need to set a lock explicitly, because this already happened
        // in CreateFile().

        // build data to write in memory
        const String data = encode(map);

        // We don't need to explicitly truncate the file, because this was done
        // with the CREATE_ALWAYS flag in the call to CreateFile().

        // Now, we can write the data to file.
        DWORD numBytesWritten;
        bool ok = WriteFile(fd, data.constData(), data.size(), &numBytesWritten,
                            NULL);
        if(ok && numBytesWritten != data.size()) ok = false;

        CloseHandle(fd);

        if(!ok)
        {
            // Delete the file on failure.
            path.rm();
        }

        return ok ? data.size() : 0;
#else
        // try to open the file
        int fd = open(path.constData(), O_RDWR|O_CREAT, 0644);
        if (fd == -1) {
            // could not open file. Maybe the parent dir does not exist?
            if (!Path::mkdir(path.parentDir(), Path::Recursive)) {
                // couldn't create parent dir either :(
                return 0;
            }

            // Parent dir creation was successful. Try to open again...
            fd = open(path.constData(), O_RDWR|O_CREAT, 0644);
            if (fd == -1) {
                // Still no luck
                return 0;
            }
        }

        // Try to aquire a write lock if requested
        if (!(options & NoLock)) {
            // use same lock as in rct's MemoryMappedFile
            int lockRes;
            eintrwrap(lockRes, flock(fd, LOCK_EX | LOCK_NB));
            if(lockRes == -1)
            {
                close(fd);
                return 0;
            }
        }

        // build data to write in memory
        const String data = encode(map);

        bool ok = ::ftruncate(fd, data.size()) != -1;
        if (!ok) {
            // could not truncate the file. Unwind file locking and creation.
            if (!(options & NoLock))
                lock(fd, Unlock);
            ::close(fd);
            return 0;
        }

        // Now, we can write the data to file.
        ok = ::write(fd, data.constData(), data.size()) == static_cast<ssize_t>(data.size());

        // If there is a lock, it is automatically released when closing fd.
        ::close(fd);

        // Delete the file on failure
        if (!ok)
            unlink(path.constData());
        return ok ? data.size() : 0;
#endif
    }
private:

#ifndef _WIN32
    enum Mode {
        Read = F_RDLCK,
        Write = F_WRLCK,
        Unlock = F_UNLCK
    };
    static bool lock(int fd, Mode mode)
    {
        struct flock fl;
        memset(&fl, 0, sizeof(fl));
        fl.l_type = mode;
        fl.l_whence = SEEK_SET;
        fl.l_pid = getpid();
        int ret;
        eintrwrap(ret, fcntl(fd, F_SETLKW, &fl));
        return ret != -1;
    }
#endif

    const char *valuesSegment() const { return mFile.filePtr<char>() + mValuesOffset; }
    const char *keysSegment() const
    {
        return mFile.filePtr<char>() + (sizeof(uint32_t) * 2);
    }

    template <typename T>
    inline T read(const char *base, uint32_t index) const
    {
        if (const uint32_t size = FixedSize<T>::value) {
            T t = T();
            memcpy(&t, base + (index * size), FixedSize<T>::value);
            return t;
        }
        uint32_t offset;
        memcpy(&offset, base + (sizeof(uint32_t) * index), sizeof(offset));
        Deserializer deserializer(mFile.filePtr<char>() + offset, INT_MAX);
        T t;
        deserializer >> t;
        return t;
    }

    MemoryMappedFile mFile;
    uint32_t mCount;
    uint32_t mValuesOffset;
    uint32_t mOptions;
};

#endif
