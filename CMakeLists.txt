# BitFunnel

cmake_minimum_required(VERSION 2.6)

# Remove the warning that WIN32 is not defined in Cygwin any more.
set(CMAKE_LEGACY_CYGWIN_WIN32 0)

project(BitFunnel_CMake)

###############################################################################
# From http://crascit.com/2015/07/25/cmake-gtest/

# Download and unpack googletest at configure time
configure_file(CMakeLists-googletest.txt.in
               googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )

# Googletest will not compile under MinGW without this.
if(MINGW)
  set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)
endif()

enable_testing()

# Add googletest directly to our build. This adds
# the following targets: gtest, gtest_main, gmock
# and gmock_main
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                 ${CMAKE_BINARY_DIR}/googletest-build)

include_directories(
  ${gtest_SOURCE_DIR}/include
)

###############################################################################

string(COMPARE EQUAL ${CMAKE_SYSTEM_NAME} "Linux" LINUX)

if (WIN32 OR CYGWIN)
  set(BITFUNNEL_PLATFORM_POSIX 0)
  set(BITFUNNEL_PLATFORM_WINDOWS 1)
else()
  set(BITFUNNEL_PLATFORM_POSIX 1)
  set(BITFUNNEL_PLATFORM_WINDOWS 0)
endif()


if(BITFUNNEL_PLATFORM_WINDOWS)
  # Note: -D works for both MSVC and GCC/Clang.
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} -DBITFUNNEL_PLATFORM_WINDOWS")
else()
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} -DBITFUNNEL_PLATFORM_POSIX")
endif()


if(MSVC)
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} /W4 /WX")
  set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} ${COMMON_CXX_FLAGS} /MTd")
  set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} ${COMMON_CXX_FLAGS} /MT")
elseif(CMAKE_COMPILER_IS_GNUCXX)
  # Need gnu++ instead of c++ so that GTest can access fdopen() etc.
  set(CMAKE_CXX_FLAGS "-msse4.2 -std=gnu++14 -Wall -Wextra -Werror")
else()
  set(CMAKE_CXX_FLAGS "-msse4.2 -std=c++14 -Wall -Wextra -Werror")
endif()


if(LINUX)
    find_package(Threads REQUIRED)
    if(CMAKE_USE_PTHREADS_INIT)
        # TODO: pthreads are an issue in general on linux.
        # TODO: We need a permanent recommendation for when/how to use libc++.
        # On clang for at least some platforms, we require `libc++-dev`.
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    endif()
endif()


# This allows the solution to group projects into folders like src and UnitTest.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

macro(COMBINE_FILE_LISTS)
    if (BITFUNNEL_PLATFORM_WINDOWS)
        set(CPPFILES ${CPPFILES} ${WINDOWS_CPPFILES})
        set(PUBLIC_HFILES ${PUBLIC_HFILES} ${WINDOWS_PUBLIC_HFILES})
        set(PRIVATE_HFILES ${PRIVATE_HFILES} ${WINDOWS_PRIVATE_HFILES})
    else (BITFUNNEL_PLATFORM_WINDOWS)
        set(CPPFILES ${CPPFILES} ${POSIX_CPPFILES})
        set(PUBLIC_HFILES ${PUBLIC_HFILES} ${POSIX_PUBLIC_HFILES})
        set(PRIVATE_HFILES ${PRIVATE_HFILES} ${POSIX_PRIVATE_HFILES})
    endif (BITFUNNEL_PLATFORM_WINDOWS)
endmacro()

###############################################################################

# TODO: add missing .h files from recent change.

set(BITFUNNEL_HFILES
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Exceptions.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/IFileManager.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/IInterface.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/IEnumerable.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/IEnumerator.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/NonCopyable.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Token.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Allocators/IAllocator.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Configuration/Factories.h
)

set(INDEX_HFILES
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/DocumentHandle.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/Factories.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/IConfiguration.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/IDocument.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/IDocumentDataSchema.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/IFactSet.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/IIngestor.h
)

set(UTILITIES_HFILES
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/Factories.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/FileHeader.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/IBlockAllocator.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/IInputStream.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/ITaskDistributor.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/ITaskProcessor.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/IThreadManager.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/RingBuffer.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/StandardInputStream.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/StreamUtilities.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/Version.h
)

set(LOGGER_HFILES
  ${CMAKE_SOURCE_DIR}/inc/LoggerInterfaces/ConsoleLogger.h
  ${CMAKE_SOURCE_DIR}/inc/LoggerInterfaces/ILogger.h
  ${CMAKE_SOURCE_DIR}/inc/LoggerInterfaces/Logging.h
  ${CMAKE_SOURCE_DIR}/inc/LoggerInterfaces/LogLevel.h
)

set(PUBLIC_HFILES
  ${BITFUNNEL_HFILES}
  ${INDEX_HFILES}
  ${UTILITIES_HFILES}
  ${LOGGER_HFILES}
)

COMBINE_FILE_LISTS()

# Public header files
add_custom_target(inc SOURCES ${PUBLIC_HFILES})
set_property(TARGET inc PROPERTY FOLDER "")
source_group("BitFunnel" FILES ${BITFUNNEL_HFILES})
source_group("BitFunnel\\Index" FILES ${INDEX_HFILES})
source_group("BitFunnel\\Utilities" FILES ${UTILITIES_HFILES})
source_group("Logger" FILES ${LOGGER_HFILES})

#
# Local package include directories.
#
include_directories(
  inc
  src/Common/CmdLineParser/inc
  src/Common/CsvTsv/inc
)

#
# Libraries and executables.
#
add_subdirectory(src)
# add_subdirectory(src/Common/CmdLineParser)
# add_subdirectory(src/Common/CsvTsv)
# add_subdirectory(src/Common/Utilities)
# add_subdirectory(src/Index)

add_subdirectory(tools/StatisticsBuilder)


add_custom_target(TOPLEVEL SOURCES
#  CMakeLists-googletest.txt.in
#  Configure_Make.bat
#  Configure_Make.sh
#  Configure_MSVC.bat
#  Configure_XCode.bat
  LICENSE.txt
  README.md
)

# TODO: CMake is making this directory, which the tests require, but we should
# ideally write platform-independent C++ code to do this.
file(MAKE_DIRECTORY "/tmp/chunks")
