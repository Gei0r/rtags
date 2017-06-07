#include "windows_mktemp.h"
#include <rct/WindowsUnicodeConversion.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <windows.h>

static const char letters[] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

int windows_mkdtemp(char *tmpl)
{
    int len;
    char *XXXXXX;
    static unsigned long long value;
    unsigned long long random_time_bits;
    unsigned int count;
    int result = -1;
    int save_errno = errno;

    // A lower bound on the number of temporary files to attempt to
    // generate.  The maximum total number of temporary file names that
    // can exist for a given tmplate is 62**6.  It should never be
    // necessary to try all these combinations.  Instead if a reasonable
    // number of names is tried (we define reasonable as 62**3) fail to
    // give the system administrator the chance to remove the problems.
    enum { ATTEMPTS_MIN = (62 * 62 * 62) };

    // The number of times to attempt to generate a temporary file.  To
    // conform to POSIX, this must be no smaller than TMP_MAX.
    enum { ATTEMPTS = ATTEMPTS_MIN < TMP_MAX ? TMP_MAX : ATTEMPTS_MIN };

    len = strlen (tmpl);
    if (len < 6 || strcmp (&tmpl[len - 6], "XXXXXX"))
    {
        errno = EINVAL;
        return -1;
    }

    // This is where the Xs start.
    XXXXXX = &tmpl[len - 6];

    // Get some more or less random data.
    {
        SYSTEMTIME      stNow;
        FILETIME        ftNow;

        // get system time
        GetSystemTime(&stNow);
        stNow.wMilliseconds = 500;
        if (!SystemTimeToFileTime(&stNow, &ftNow))
        {
            errno = -1;
            return -1;
        }

        random_time_bits = (((unsigned long long)ftNow.dwHighDateTime << 32)
                            | (unsigned long long)ftNow.dwLowDateTime);
    }
    value += random_time_bits ^ (unsigned long long)GetCurrentThreadId ();

    for (count = 0; count < ATTEMPTS; value += 7777, ++count)
    {
        unsigned long long v = value;

        // Fill in the random bits.
        XXXXXX[0] = letters[v % 62];
        v /= 62;
        XXXXXX[1] = letters[v % 62];
        v /= 62;
        XXXXXX[2] = letters[v % 62];
        v /= 62;
        XXXXXX[3] = letters[v % 62];
        v /= 62;
        XXXXXX[4] = letters[v % 62];
        v /= 62;
        XXXXXX[5] = letters[v % 62];

        Utf8To16 wtmpl(tmpl);
        result = _wmkdir(wtmpl.asWchar_t());
        if (result >= 0)
        {
            errno = save_errno;
            return result;
        }
        else if (errno != EEXIST)
            return -1;
    }

    // We got out of the loop because we ran out of combinations to try.
    errno = EEXIST;
    return -1;
}

int windows_mkstemp(char *tmpl)
{
    return windows_mkostemps(
        tmpl,
        0,
        O_RDWR | O_CREAT | O_EXCL);
}

int windows_mkostemp(char *tmpl, int flags)
{
    return windows_mkostemps(
        tmpl,
        0,
        flags);
}

int windows_mkstemps(char *tmpl, int suffixlen)
{
    return windows_mkostemps(
        tmpl,
        suffixlen,
        O_RDWR | O_CREAT | O_EXCL);
}

int windows_mkostemps(char *tmpl, int suffixlen, int flags)
{
    int len;
    char *XXXXXX;
    static unsigned long long value;
    unsigned long long random_time_bits;
    unsigned int count;
    int fd = -1;
    int save_errno = errno;
    int xs_begin_pos;

    // A lower bound on the number of temporary files to attempt to
    // generate.  The maximum total number of temporary file names that
    // can exist for a given tmplate is 62**6.  It should never be
    // necessary to try all these combinations.  Instead if a reasonable
    // number of names is tried (we define reasonable as 62**3) fail to
    // give the system administrator the chance to remove the problems.
    enum { XS_COUNT = 6};
    enum { ATTEMPTS_MIN = (62 * 62 * 62) };

    // The number of times to attempt to generate a temporary file.  To
    // conform to POSIX, this must be no smaller than TMP_MAX.
    enum { ATTEMPTS = ATTEMPTS_MIN < TMP_MAX ? TMP_MAX : ATTEMPTS_MIN };

    len = strlen (tmpl);
    xs_begin_pos = len - 6 - suffixlen;
    if (xs_begin_pos < 0 || strcmp (&tmpl[xs_begin_pos], "XXXXXX"))
    {
        errno = EINVAL;
        return -1;
    }

    // This is where the Xs start.
    XXXXXX = &tmpl[xs_begin_pos];

    // Get some more or less random data.
    {
        SYSTEMTIME      stNow;
        FILETIME        ftNow;

        // get system time
        GetSystemTime(&stNow);
        stNow.wMilliseconds = 500;
        if (!SystemTimeToFileTime(&stNow, &ftNow))
        {
            errno = -1;
            return -1;
        }

        random_time_bits = (((unsigned long long)ftNow.dwHighDateTime << 32)
                            | (unsigned long long)ftNow.dwLowDateTime);
    }
    value += random_time_bits ^ (unsigned long long)GetCurrentThreadId ();

    for (count = 0; count < ATTEMPTS; value += 7777, ++count)
    {
        unsigned long long v = value;

        // Fill in the random bits.
        XXXXXX[0] = letters[v % 62];
        v /= 62;
        XXXXXX[1] = letters[v % 62];
        v /= 62;
        XXXXXX[2] = letters[v % 62];
        v /= 62;
        XXXXXX[3] = letters[v % 62];
        v /= 62;
        XXXXXX[4] = letters[v % 62];
        v /= 62;
        XXXXXX[5] = letters[v % 62];

        Utf8To16 wtmpl(tmpl);
        fd = _wopen(wtmpl.asWchar_t(), flags, _S_IREAD | _S_IWRITE);
        if (fd >= 0)
        {
            errno = save_errno;
            return fd;
        }
        else if (errno != EEXIST)
            return -1;
    }

    // We got out of the loop because we ran out of combinations to try.
    errno = EEXIST;
    return -1;
}
