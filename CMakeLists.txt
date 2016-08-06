cmake_minimum_required(VERSION 2.8.6)
project(rtags)
set(RTAGS_VERSION_MAJOR 2)
set(RTAGS_VERSION_MINOR 4)
set(RTAGS_VERSION_DATABASE 100)
set(RTAGS_VERSION_SOURCES_FILE 6)
set(RTAGS_VERSION ${RTAGS_VERSION_MAJOR}.${RTAGS_VERSION_MINOR}.${RTAGS_VERSION_DATABASE})

set(CMAKE_LEGACY_CYGWIN_WIN32 0)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_COMPILER_IS_CLANGCXX TRUE)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

include(FeatureSummary)
add_subdirectory(src)

if (EXISTS "rules.ninja")
    add_custom_target(release COMMAND ${CMAKE_COMMAND} -GNinja -DCMAKE_BUILD_TYPE=Release . WORKING_DIRECTORY .)
    add_custom_target(debug COMMAND ${CMAKE_COMMAND} -GNinja -DCMAKE_BUILD_TYPE=Debug . WORKING_DIRECTORY .)
else()
    add_custom_target(release COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release . WORKING_DIRECTORY .)
    add_custom_target(debug COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug . WORKING_DIRECTORY .)
endif()

# Create uninstall script and target
configure_file("${CMAKE_MODULE_PATH}/Uninstall.cmake.in" "${CMAKE_BINARY_DIR}/Uninstall.cmake" IMMEDIATE @ONLY)
add_custom_target(uninstall ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/Uninstall.cmake")

if (RTAGS_ENABLE_DEV_OPTIONS)
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "RTAGS")
    set(CPACK_PACKAGE_VENDOR "RTags")
    set(CPACK_PACKAGE_CONTACT "http://rtags.net")
    set(CPACK_PACKAGE_VERSION_MAJOR ${RTAGS_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${RTAGS_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_DATABASE ${RTAGS_VERSION_DATABASE})
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
    set(CPACK_PACKAGE_VERSION "${RTAGS_VERSION}")
    include(CPack)
endif ()

feature_summary(INCLUDE_QUIET_PACKAGES WHAT ALL)

include(ExternalProject)

if (RTAGS_BUILD_CLANG)
    ExternalProject_Add(llvmclang
        DOWNLOAD_COMMAND curl http://llvm.org/releases/3.8.1/llvm-3.8.1.src.tar.xz -o llvm-3.8.1.src.tar.xz && tar xf llvm-3.8.1.src.tar.xz && curl http://llvm.org/releases/3.8.1/cfe-3.8.1.src.tar.xz -o llvm-3.8.1.src/tools/cfe-3.8.1.src.tar.xz && tar xf llvm-3.8.1.src/tools/cfe-3.8.1.src.tar.xz -C llvm-3.8.1.src/tools && rm -rf llvm-3.8.1.src/tools/clang && mv llvm-3.8.1.src/tools/cfe-3.8.1.src llvm-3.8.1.src/tools/clang
        SOURCE_DIR llvmclang-prefix/src/llvm-3.8.1.src
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
        LOG_BUILD 1
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
