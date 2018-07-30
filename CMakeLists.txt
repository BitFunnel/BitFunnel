# BitFunnel

cmake_minimum_required(VERSION 2.6)

# Remove the warning that WIN32 is not defined in Cygwin any more.
set(CMAKE_LEGACY_CYGWIN_WIN32 0)

# PRoject-wide settings for BitFunnel
project(BitFunnel_CMake)

################################################################################

# # gtest

# add_subdirectory(googletest)

# include_directories(
#   ${gtest_SOURCE_DIR}/include
# )

################################################################################

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


if (WIN32 OR CYGWIN)
    set(NATIVEJIT_PLATFORM_POSIX 0)
    set(NATIVEJIT_PLATFORM_WINDOWS 1)
else()
    set(NATIVEJIT_PLATFORM_POSIX 1)
    set(NATIVEJIT_PLATFORM_WINDOWS 0)
endif()


if(NATIVEJIT_PLATFORM_WINDOWS)
  # Note: -D works for both MSVC and GCC/Clang.
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} -DNATIVEJIT_PLATFORM_WINDOWS")
else()
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} -DNATIVEJIT_PLATFORM_POSIX")
endif()


# TODO: consider fixing weak-vtables.
# TODO: fix sign-conversion. There's a lot in CmdLineParser.
# TODO: can we fix format-non-literal?
# TODO: we can remove -Wno-switch-enum when we get rid of StreamId as an enum.
# TODO: only disable shorten-64-to-32 where we check for overflow and then throw. This is done in RowId.
# TODO: see issue #147 about non-virtual destructor.
# Wno-missing-prototypes is because we commonly declare functions without prototypes in tests.
# Wno-float-equal is because we have tests for serializing and deserialization of floats that compare exact values.
# Wno-exit-time-destructors is for Logging
# Wno-switch-enum is because we often have a "default" case that throws for all invalid types.
set(GTEST_REQUIRED_FLAGS "-Wno-shift-sign-overflow -Wno-missing-noreturn -Wno-used-but-marked-unused -Wno-deprecated -Wno-missing-variable-declarations -Wno-global-constructors -Wno-weak-vtables")
set(WEVERYTHING_TODO_FLAGS "-Wno-format-nonliteral -Wno-non-virtual-dtor")
set(WEVERYTHING_FLAGS "-Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-undef -Wno-missing-prototypes -Wno-float-equal -Wno-exit-time-destructors -Wno-switch-enum ${GTEST_REQUIRED_FLAGS} ${WEVERYTHING_TODO_FLAGS}")
# The standard way to use msan is to compile libc++ with msan.
# TODO: figure out how to make msan linker flags generic.
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -L/home/leah/dev/libcxx_msan/lib -lc++abi -I/home/leah/dev/libcxx_msan/include -I/home/leah/dev/libcxx_msan/include/c++/v1")
# set(MSAN_FLAGS "-fsanitize=memory")

if(MSVC)
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} /W4 /WX -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING")
  set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} ${COMMON_CXX_FLAGS} /MTd")
  set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} ${COMMON_CXX_FLAGS} /MT /Z7")
  set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /DEBUG" )
elseif(CMAKE_COMPILER_IS_GNUCXX)
  # Need gnu++ instead of c++ so that GTest can access fdopen() etc.
  set(CMAKE_CXX_FLAGS "-msse4.2 -std=gnu++14 -Wall -Wextra -Werror -Wold-style-cast -fstrict-aliasing -Wstrict-aliasing")
else()
  # TODO: define a target for -Weverything.
  # set(CMAKE_CXX_FLAGS "-msse4.2 -std=c++14 -Wall -Wextra -Werror -Wold-style-cast ${WEVERYTHING_FLAGS} -Wno-missing-braces")
  # TODO: define a target for msan.
  # set(CMAKE_CXX_FLAGS "-msse4.2 -std=c++14 -Wall -Wextra -Werror -Wold-style-cast ${MSAN_FLAGS} -Wno-missing-braces")
  set(CMAKE_CXX_FLAGS "-msse4.2 -std=c++14 -Wall -Wextra -Werror -Wold-style-cast -Wno-missing-braces")
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


# This allows the solution to group projects into folders like src and test.
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

set(BITFUNNEL_HFILES
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/BitFunnelTypes.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Exceptions.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/IDiagnosticStream.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/IEnumerable.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/IEnumerator.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/IExecutable.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/IFileManager.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/IInterface.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/NonCopyable.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Term.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Allocators/IAllocator.h
)

set(CONFIGURATION_HFILES
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Configuration/Factories.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Configuration/IFileSystem.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Configuration/IStreamConfiguration.h
)

set(CHUNKS_HFILES
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Chunks/DocumentFilters.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Chunks/IChunkManifestIngestor.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Chunks/IChunkProcessor.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Chunks/IChunkWriter.h
)

set(DATA_HFILES
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Data/Sonnets.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Data/SyntheticChunks.h
)

set(INDEX_HFILES
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/DocumentHandle.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/Factories.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/Helpers.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/IConfiguration.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/ICostFunction.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/IDocument.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/IDocumentCache.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/IDocumentDataSchema.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/IDocumentHistogram.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/IFactSet.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/IIngestor.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/IngestChunks.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/IRecycler.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/IShard.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/IShardCostFunction.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/ISimpleIndex.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/ISliceBufferAllocator.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/ITermTable.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/ITermTableCollection.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/ITermTreatment.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/ITermTreatmentFactory.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/ITermToText.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/PackedRowIdSequence.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/Row.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/RowId.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/RowIdSequence.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/Token.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/ShardDefinitionBuilder.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Index/Token.h
)

set(MOCKS_HFILES
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Mocks/Factories.h
)

set(PLAN_HFILES
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Plan/Factories.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Plan/IMatchVerifier.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Plan/IQueryEngine.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Plan/QueryInstrumentation.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Plan/QueryParser.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Plan/QueryRunner.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Plan/ResultsBuffer.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Plan/TermMatchNode.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Plan/VerifyOneQuery.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Plan/VerifyOneQuerySynthetic.h
)

set(UTILITIES_HFILES
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/Accumulator.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/Allocator.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/BlockingQueue.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/Factories.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/Exists.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/FileHeader.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/IBlockAllocator.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/IInputStream.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/IObjectFormatter.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/IObjectParser.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/IPersistableObject.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/IsSpace.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/ITaskDistributor.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/ITaskProcessor.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/IThreadManager.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/Primes.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/Random.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/ReadLines.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/RingBuffer.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/StandardInputStream.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/Stopwatch.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/StreamUtilities.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/StringBuilder.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/TextObjectFormatter.h
  ${CMAKE_SOURCE_DIR}/inc/BitFunnel/Utilities/Version.h
)

set(CSVTSV_HFILES
  ${CMAKE_SOURCE_DIR}/inc/CsvTsv/Csv.h
  ${CMAKE_SOURCE_DIR}/inc/CsvTsv/DelimitedTable.h
  ${CMAKE_SOURCE_DIR}/inc/CsvTsv/ParseError.h
  ${CMAKE_SOURCE_DIR}/inc/CsvTsv/Table.h
  ${CMAKE_SOURCE_DIR}/inc/CsvTsv/Tsv.h
)

set(LOGGER_HFILES
  ${CMAKE_SOURCE_DIR}/inc/LoggerInterfaces/Check.h
  ${CMAKE_SOURCE_DIR}/inc/LoggerInterfaces/ConsoleLogger.h
  ${CMAKE_SOURCE_DIR}/inc/LoggerInterfaces/ILogger.h
  ${CMAKE_SOURCE_DIR}/inc/LoggerInterfaces/Logging.h
  ${CMAKE_SOURCE_DIR}/inc/LoggerInterfaces/LogLevel.h
)

set(PUBLIC_HFILES
  ${BITFUNNEL_HFILES}
  ${CONFIGURATION_HFILES}
  ${CHUNKS_HFILES}
  ${DATA_HFILES}
  ${INDEX_HFILES}
  ${MOCKS_HFILES}
  ${PLAN_HFILES}
  ${UTILITIES_HFILES}
  ${CSVTSV_HFILES}
  ${LOGGER_HFILES}
)

COMBINE_FILE_LISTS()

# Public header files
add_custom_target(inc SOURCES ${PUBLIC_HFILES})
set_property(TARGET inc PROPERTY FOLDER "")
source_group("BitFunnel" FILES ${BITFUNNEL_HFILES})
source_group("BitFunnel\\Configuration" FILES ${CONFIGURATION_HFILES})
source_group("BitFunnel\\Chunks" FILES ${CHUNKS_HFILES})
source_group("BitFunnel\\Data" FILES ${DATA_HFILES})
source_group("BitFunnel\\Index" FILES ${INDEX_HFILES})
source_group("BitFunnel\\Mocks" FILES ${MOCKS_HFILES})
source_group("BitFunnel\\Plan" FILES ${PLAN_HFILES})
source_group("BitFunnel\\Utilities" FILES ${UTILITIES_HFILES})
source_group("Logger" FILES ${LOGGER_HFILES})

#
# Local package include directories.
#
include_directories(
  inc
  NativeJIT/inc
  src/Common/CmdLineParser/inc
  src/Common/CsvTsv/inc
)

enable_testing()

#
# Libraries and executables.
#
add_subdirectory(NativeJIT)

add_subdirectory(examples/QueryParser)
add_subdirectory(src)
add_subdirectory(test/Shared)
add_subdirectory(tools/BitFunnel)
add_subdirectory(tools/CsvExtract)

add_custom_target(TOPLEVEL SOURCES
#  Configure_Make.bat
#  Configure_Make.sh
#  Configure_MSVC.bat
#  Configure_XCode.bat
  LICENSE.txt
  README.md
)

set(ENV{GTEST_COLOR} 1)
set(ENV{GTEST_CATCH_EXCEPTIONS} 0)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --verbose)
