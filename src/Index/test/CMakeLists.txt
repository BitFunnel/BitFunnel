# BitFunnel/src/Index/test

set(CPPFILES
    ChunkReaderTest.cpp
    ChunkTaskProcessorTest.cpp
    DocTableDescriptorTest.cpp
    DocumentDataSchemaTest.cpp
    DocumentHandleTest.cpp
    DocumentLengthHistogramTest.cpp
    IndexUtilsTest.cpp
    IngestorTest.cpp
    RowTableDescriptorTest.cpp
    ShardTest.cpp
    SliceTest.cpp
    TrackingSliceBufferAllocator.cpp
)

set(WINDOWS_CPPFILES
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
    TrackingSliceBufferAllocator.h
)

set(WINDOWS_PRIVATE_HFILES
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()

# Unit tests are allowed to access private headers of the library they test.
include_directories(${CMAKE_SOURCE_DIR}/src/Index/src)

# TODO: fix this hack.
include_directories(${CMAKE_SOURCE_DIR}/test/Shared)

add_executable(IndexTest ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET IndexTest PROPERTY FOLDER "src/Index")
set_property(TARGET IndexTest PROPERTY PROJECT_LABEL "Test")

# NOTE: The ordering Utilities-Index is important for XCode. If you reverse
# Utilities and Index, we will get linker errors.
target_link_libraries (IndexTest CsvTsv TestShared Utilities Index Configuration CsvTsv Utilities  gtest gtest_main)

add_test(NAME IndexTest COMMAND IndexTest)
