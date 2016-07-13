# BitFunnel/src/Index/UnitTest

set(CPPFILES
    ChunkReaderUnitTest.cpp
    ChunkTaskProcessorUnitTest.cpp
    DocTableDescriptorUnitTest.cpp
    DocumentDataSchemaUnitTest.cpp
    DocumentLengthHistogramUnitTest.cpp
    Mocks/EmptyTermTable.cpp
    # RecyclerUnitTest.cpp
    RowTableDescriptorUnitTest.cpp
    # ShardUnitTest.cpp
    SliceUnitTest.cpp
    TrackingSliceBufferAllocator.cpp
)

set(WINDOWS_CPPFILES
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
    Mocks/ChunkEventTracer.h
    Mocks/EmptyTermTable.h
    TrackingSliceBufferAllocator.h
)

set(WINDOWS_PRIVATE_HFILES
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()

# Unit tests are allowed to access private headers of the library they test.
include_directories(${CMAKE_SOURCE_DIR}/src/Index/src)


add_executable(IndexUnitTest ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET IndexUnitTest PROPERTY FOLDER "src/Index")
set_property(TARGET IndexUnitTest PROPERTY PROJECT_LABEL "UnitTest")

# NOTE: The ordering Utilities-Index is important for XCode. If you reverse
# Utilities and Index, we will get linker errors.
target_link_libraries (IndexUnitTest CsvTsv Utilities Index CsvTsv Utilities gtest gtest_main)

add_test(NAME IndexUnitTest COMMAND IndexUnitTest)
