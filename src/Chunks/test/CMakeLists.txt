# BitFunnel/src/Chunks/test

set(CPPFILES
    ChunkReaderTest.cpp
    DocumentTest.cpp
)

set(WINDOWS_CPPFILES
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
    ChunkEventTracer.h
)

set(WINDOWS_PRIVATE_HFILES
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()

# Unit tests are allowed to access private headers of the library they test.
include_directories(${CMAKE_SOURCE_DIR}/src/Chunks/src)

# TODO: fix this hack.
include_directories(${CMAKE_SOURCE_DIR}/test/Shared)

add_executable(ChunksTest ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET ChunksTest PROPERTY FOLDER "src/Chunks")
set_property(TARGET ChunksTest PROPERTY PROJECT_LABEL "Test")

# NOTE: The ordering Utilities-Index is important for XCode. If you reverse
# Utilities and Index, we will get linker errors.
target_link_libraries (ChunksTest Chunks Index CsvTsv Utilities gtest gtest_main)

add_test(NAME ChunksTest COMMAND ChunksTest)
