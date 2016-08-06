set(CMAKE_SKIP_BUILD_RPATH            FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH    FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

include(ExternalProject)

if (RTAGS_BUILD_CLANG)
    ExternalProject_Add(llvmclang
        DOWNLOAD_COMMAND ${PROJECT_SOURCE_DIR}/scripts/getclang.sh
        # DOWNLOAD_COMMAND curl -z llvm-3.8.1.src.tar.xz http://llvm.org/releases/3.8.1/llvm-3.8.1.src.tar.xz -o llvm-3.8.1.src.tar.xz && tar xf llvm-3.8.1.src.tar.xz && curl -z cfe-3.8.1.src.tar.xz http://llvm.org/releases/3.8.1/cfe-3.8.1.src.tar.xz -o llvm-3.8.1.src/tools/cfe-3.8.1.src.tar.xz && tar xf llvm-3.8.1.src/tools/cfe-3.8.1.src.tar.xz -C llvm-3.8.1.src/tools && rm -rf llvm-3.8.1.src/tools/clang && mv llvm-3.8.1.src/tools/cfe-3.8.1.src llvm-3.8.1.src/tools/clang
        SOURCE_DIR llvmclang-prefix/src/llvm-3.8.1.src
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
        INSTALL_COMMAND ""
        LOG_DOWNLOAD 1            # Wrap download in script to log output
        LOG_UPDATE 1              # Wrap update in script to log output
        LOG_CONFIGURE 1           # Wrap configure in script to log output
        LOG_BUILD 1               # Wrap build in script to log output
        LOG_TEST 1                # Wrap test in script to log output
        LOG_INSTALL 1             # Wrap install in script to log output
        )
endif ()

#ExternalProject_Add(<name>    # Name for custom target
#  [DEPENDS projects...]       # Targets on which the project depends
#  [PREFIX dir]                # Root dir for entire project
#  [LIST_SEPARATOR sep]        # Sep to be replaced by ; in cmd lines
#  [TMP_DIR dir]               # Directory to store temporary files
#  [STAMP_DIR dir]             # Directory to store step timestamps
# #--Download step--------------
#  [DOWNLOAD_NAME fname]       # File name to store (if not end of URL)
#  [DOWNLOAD_DIR dir]          # Directory to store downloaded files
#  [DOWNLOAD_COMMAND cmd...]   # Command to download source tree
#  [CVS_REPOSITORY cvsroot]    # CVSROOT of CVS repository
#  [CVS_MODULE mod]            # Module to checkout from CVS repo
#  [CVS_TAG tag]               # Tag to checkout from CVS repo
#  [SVN_REPOSITORY url]        # URL of Subversion repo
#  [SVN_REVISION rev]          # Revision to checkout from Subversion repo
#  [SVN_USERNAME john ]        # Username for Subversion checkout and update
#  [SVN_PASSWORD doe ]         # Password for Subversion checkout and update
#  [SVN_TRUST_CERT 1 ]         # Trust the Subversion server site certificate
#  [GIT_REPOSITORY url]        # URL of git repo
#  [GIT_TAG tag]               # Git branch name, commit id or tag
#  [GIT_SUBMODULES modules...] # Git submodules that shall be updated, all if empty
#  [HG_REPOSITORY url]         # URL of mercurial repo
#  [HG_TAG tag]                # Mercurial branch name, commit id or tag
#  [URL /.../src.tgz]          # Full path or URL of source
#  [URL_HASH ALGO=value]       # Hash of file at URL
#  [URL_MD5 md5]               # Equivalent to URL_HASH MD5=md5
#  [TLS_VERIFY bool]           # Should certificate for https be checked
#  [TLS_CAINFO file]           # Path to a certificate authority file
#  [TIMEOUT seconds]           # Time allowed for file download operations
# #--Update/Patch step----------
#  [UPDATE_COMMAND cmd...]     # Source work-tree update command
#  [PATCH_COMMAND cmd...]      # Command to patch downloaded source
# #--Configure step-------------
#  [SOURCE_DIR dir]            # Source dir to be used for build
#  [CONFIGURE_COMMAND cmd...]  # Build tree configuration command
#  [CMAKE_COMMAND /.../cmake]  # Specify alternative cmake executable
#  [CMAKE_GENERATOR gen]       # Specify generator for native build
#  [CMAKE_GENERATOR_TOOLSET t] # Generator-specific toolset name
#  [CMAKE_ARGS args...]        # Arguments to CMake command line
#  [CMAKE_CACHE_ARGS args...]  # Initial cache arguments, of the form -Dvar:string=on
# #--Build step-----------------
#  [BINARY_DIR dir]            # Specify build dir location
#  [BUILD_COMMAND cmd...]      # Command to drive the native build
#  [BUILD_IN_SOURCE 1]         # Use source dir for build dir
# #--Install step---------------
#  [INSTALL_DIR dir]           # Installation prefix
#  [INSTALL_COMMAND cmd...]    # Command to drive install after build
# #--Test step------------------
#  [TEST_BEFORE_INSTALL 1]     # Add test step executed before install step
#  [TEST_AFTER_INSTALL 1]      # Add test step executed after install step
#  [TEST_COMMAND cmd...]       # Command to drive test
# #--Output logging-------------
#  [LOG_DOWNLOAD 1]            # Wrap download in script to log output
#  [LOG_UPDATE 1]              # Wrap update in script to log output
#  [LOG_CONFIGURE 1]           # Wrap configure in script to log output
#  [LOG_BUILD 1]               # Wrap build in script to log output
#  [LOG_TEST 1]                # Wrap test in script to log output
#  [LOG_INSTALL 1]             # Wrap install in script to log output
# #--Custom targets-------------
#  [STEP_TARGETS st1 st2 ...]  # Generate custom targets for these steps
#  )
#

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/RTagsVersion.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/RTagsVersion.h"
    )

# Treat MSYS as Cygwin
if (MSYS)
    set(CYGWIN 1)
    set(CMAKE_SYSTEM_NAME "CYGWIN")
endif ()

# Set RPATH when installing to a custom (non-system) directory
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
if ("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:${CMAKE_INSTALL_PREFIX}/lib")
endif ()

include(${CMAKE_SOURCE_DIR}/cmake/EnsureLibraries.cmake)

if (NOT DEFINED LIBCLANG_LIBRARIES)
    set(LIBCLANG_LIBRARIES $ENV{LIBCLANG_LIBRARIES})
endif ()

if (NOT DEFINED LIBCLANG_CXXFLAGS)
    set(LIBCLANG_CXXFLAGS $ENV{LIBCLANG_CXXFLAGS})
endif ()

if (NOT DEFINED LIBCLANG_LIBDIR)
    set(LIBCLANG_LIBDIR $ENV{LIBCLANG_LIBDIR})
endif()

if (NOT DEFINED LIBCLANG_LIBRARIES OR NOT DEFINED LIBCLANG_CXXFLAGS OR NOT DEFINED LIBCLANG_LIBDIR)
    find_package(LibClang REQUIRED)
endif ()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LIBCLANG_CXXFLAGS}")

if (EXISTS ${CLANG_COMPILATION_INCLUDE})
    add_definitions(-DHAVE_CXCOMPILATIONDATABASE)
endif ()

find_package(Lua 5.3 QUIET)
set_package_properties(Lua
    PROPERTIES
    URL "https://www.lua.org/"
    DESCRIPTION "a scripting language"
    TYPE OPTIONAL
    PURPOSE "Dependency of the selene sub-module, see http://www.rtags.net for a more detailed description.")
if (LUA_FOUND)
    set(RCT_RTTI_ENABLED 1)
endif ()

set(RCT_NO_INSTALL 1)
set(RCT_NO_LIBRARY 1)
# Everyting which as been set either in rct/rct.cmake or rct/compiler.cmake
# doesn't need to be set in this file again.
include(rct/rct.cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wstrict-aliasing=2 -Wcast-qual -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wstrict-aliasing=2 -Wcast-qual -fPIC")
if (NOT CYGWIN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -Wstack-protector")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all -Wstack-protector")
endif ()

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions("-DRTAGS_DEBUG")
    set(RCT_EVENTLOOP_CALLBACK_TIME_THRESHOLD 2000)
endif ()

add_definitions(
    "-DRTAGS_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
    "-DCLANG_LIBDIR=${LIBCLANG_LIBDIR}"
    "-DOS_${CMAKE_SYSTEM_NAME}"
    ${RCT_DEFINITIONS})

if (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    add_definitions(-D__LONG_LONG_SUPPORTED)
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

set(RTAGS_SOURCES
    ClangIndexer.cpp
    ClangThread.cpp
    ClassHierarchyJob.cpp
    CompilerManager.cpp
    CompletionThread.cpp
    DependenciesJob.cpp
    FileManager.cpp
    FindFileJob.cpp
    FindSymbolsJob.cpp
    FollowLocationJob.cpp
    IncludeFileJob.cpp
    IndexMessage.cpp
    IndexerJob.cpp
    JobScheduler.cpp
    ListSymbolsJob.cpp
    Location.cpp
    Preprocessor.cpp
    Project.cpp
    QueryJob.cpp
    QueryMessage.cpp
    RClient.cpp
    RTags.cpp
    ReferencesJob.cpp
    Sandbox.cpp
    ScanThread.cpp
    Server.cpp
    Source.cpp
    StatusJob.cpp
    Symbol.cpp
    Symbol.cpp
    SymbolInfoJob.cpp
    Token.cpp
    TokensJob.cpp
    ${RCT_SOURCES})

if (LUA_FOUND)
    list(APPEND RTAGS_SOURCES AST.cpp)
endif ()

add_library(rtags STATIC ${RTAGS_SOURCES})
if (RTAGS_COTIRE)
    include(cotire)
    set_target_properties(rtags PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)
    cotire(rtags)
endif ()

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
    ${RCT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_LIST_DIR}/selene/include)

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(START_GROUP "")
    set(END_GROUP "")
else ()
    set(START_GROUP "-Wl,--start-group")
    set(END_GROUP "-Wl,--end-group")
endif ()

set(RTAGS_LIBRARIES rtags ${START_GROUP} ${LIBCLANG_LIBRARIES} ${END_GROUP})

if (LUA_FOUND)
    set(RTAGS_LIBRARIES ${RTAGS_LIBRARIES} ${LUA_LIBRARIES})
    include_directories(${LUA_INCLUDE_DIR})
    add_definitions(-DRTAGS_HAS_LUA)
    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        set_source_files_properties(AST.cpp PROPERTIES
            COMPILE_FLAGS "-Wno-unused-parameter -Wno-unused-variable -ftemplate-depth=2000 -Wno-cast-qual")
    else ()
        set_source_files_properties(AST.cpp PROPERTIES
            COMPILE_FLAGS "-Wno-unused-but-set-parameter -Wno-unused-parameter -Wno-unused-variable -ftemplate-depth=2000 -Wno-cast-qual")
    endif()
endif ()

# RCT_LIBRARIES and stdc++ library must be at the end
set(RTAGS_LIBRARIES ${RTAGS_LIBRARIES} -lstdc++ ${RCT_LIBRARIES})
add_executable(rc rc.cpp)
target_link_libraries(rc ${RTAGS_LIBRARIES})

add_executable(rdm rdm.cpp)

target_link_libraries(rdm ${RTAGS_LIBRARIES})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

add_executable(rp rp.cpp)
target_link_libraries(rp ${RTAGS_LIBRARIES})

if (CYGWIN)
    EnsureLibraries(rdm rct)
endif ()

if (EMACS)
    set(EMACS_EXECUTABLE ${EMACS})
else ()
    find_program(EMACS_EXECUTABLE emacs)
endif ()

if (EMACS_EXECUTABLE)
    execute_process(COMMAND ${EMACS_EXECUTABLE} --version
        RESULT_VARIABLE EMACS_ERROR
        OUTPUT_VARIABLE EMACS_VERSION_INFORMATION
        ERROR_QUIET)
    if (NOT EMACS_ERROR)
        string(REGEX MATCH "[0-9]+(\\.[0-9]+)*" EMACS_VERSION ${EMACS_VERSION_INFORMATION})
        if ("${EMACS_VERSION}" LESS "23")
            message(STATUS "Installed emacs version is to old, elisp files are not going to be installed, minimum required version is >=23!")
            set(RTAGS_NO_ELISP_FILES TRUE)
        endif ()
    else ()
        message(STATUS "Could not get emacs version (\"emacs --version\"), elisp files are not going to be installed")
        set(RTAGS_NO_ELISP_FILES TRUE)
    endif ()
else ()
    set(RTAGS_NO_ELISP_FILES TRUE)
    message(STATUS "Emacs was not found, elisp files are not going to be installed!")
endif ()

set(RTAGS_ELISP_FILES
    rtags.el
    rtags-ac.el
    company-rtags.el
    flycheck-rtags.el)

if (NOT RTAGS_NO_LUA_FILES AND LUA_FOUND AND NOT ${CMAKE_CURRENT_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rtags.lua
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/rtags.lua ${CMAKE_CURRENT_BINARY_DIR}/rtags.lua
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/rtags.lua
        COMMENT "Copying ${CMAKE_CURRENT_SOURCE_DIR}/rtags.lua -> ${CMAKE_CURRENT_BINARY_DIR}/rtags.lua")
endif ()

if (NOT RTAGS_NO_ELISP_FILES)
    if (NOT RTAGS_ELISP_INSTALL_LOCATION)
        set(RTAGS_ELISP_INSTALL_LOCATION ${CMAKE_INSTALL_PREFIX}/share/emacs/site-lisp/rtags/)
    endif ()
    set(RTAGS_ELISP_SOURCES "")
    foreach(el ${RTAGS_ELISP_FILES})
        if (NOT ${CMAKE_CURRENT_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
            add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${el}
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${el} ${CMAKE_CURRENT_BINARY_DIR}/${el}
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${el}
                COMMENT "Copying ${CMAKE_CURRENT_SOURCE_DIR}/${el} -> ${CMAKE_CURRENT_BINARY_DIR}/${el}")
        endif ()
        list(APPEND RTAGS_ELISP_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/${el})
        if (NOT RTAGS_NO_ELISP_BYTECOMPILE)
            list(APPEND RTAGS_ELISP_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/${el}c)
            add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${el}c
                COMMAND ${EMACS_EXECUTABLE} -batch -l ${CMAKE_CURRENT_SOURCE_DIR}/compile-shim.elisp -l ${CMAKE_CURRENT_SOURCE_DIR}/rtags.el -f batch-byte-compile
                ${CMAKE_CURRENT_BINARY_DIR}/${el}
                DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${el}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                COMMENT "Creating byte-compiled Emacs lisp ${CMAKE_CURRENT_BINARY_DIR}/${el}c")
            add_custom_target(emacs_byte_compile_${el} ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${el}c)
        endif ()
    endforeach()
endif ()

install(CODE "message(\"Installing rtags...\")")
include(BashCompletion)
if (BASH_COMPLETION_FOUND)
    file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/completions")
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink rtags rc
        COMMAND ${CMAKE_COMMAND} -E create_symlink rtags rdm
        WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/completions")
    install(FILES rtags-bash-completion.bash DESTINATION ${BASH_COMPLETION_COMPLETIONSDIR}
        PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
        RENAME rtags)
    install(FILES "${PROJECT_BINARY_DIR}/completions/rc" "${PROJECT_BINARY_DIR}/completions/rdm"
        DESTINATION ${BASH_COMPLETION_COMPLETIONSDIR})
endif ()
install(TARGETS rdm rc rp RUNTIME DESTINATION bin COMPONENT rtags)
install(FILES ../bin/gcc-rtags-wrapper.sh DESTINATION bin
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install(FILES ../man/man7/rc.7 ../man/man7/rdm.7 DESTINATION share/man/man7/)
if (NOT RTAGS_NO_ELISP_FILES)
    install(FILES ${RTAGS_ELISP_SOURCES} DESTINATION ${RTAGS_ELISP_INSTALL_LOCATION})
endif ()
