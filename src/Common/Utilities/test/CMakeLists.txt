# BitFunnel/src/Common/Utilities/test

# TODO: move ThrowingLogger to some shared folder?

set(CPPFILES
    Array2DFixedUnitTest.cpp
    Array3DFixedUnitTest.cpp
    Array2DUnitTest.cpp
    BlockAllocatorUnitTest.cpp
    BlockingQueueUnitTest.cpp
    ConstructorDestructorCounter.cpp
    FileHeaderUnitTest.cpp
    MurmurHashUnitTest.cpp
    PackedArrayUnitTest.cpp
    RandomUnitTest.cpp
    RoundingUnitTest.cpp
    SimpleHashSetUnitTest.cpp
    SimpleHashTableUnitTest.cpp
    StreamUtilitiesUnitTest.cpp
    TaskDistributorUnitTest.cpp
    ThrowingLogger.cpp
    TokenManagerUnitTest.cpp
    TokenTrackerUnitTest.cpp
    TokenUnitTest.cpp
    VersionUnitTest.cpp
)

set(WINDOWS_CPPFILES
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
    ThrowingLogger.h
)

set(WINDOWS_PRIVATE_HFILES
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()

# Unit tests are allowed to access private headers of the library they test.
include_directories(${CMAKE_SOURCE_DIR}/src/Common/Utilities/src)


add_executable(UtilitiesTest ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET UtilitiesTest PROPERTY FOLDER "src/Common/Utilities")
set_property(TARGET UtilitiesTest PROPERTY PROJECT_LABEL "Test")
target_link_libraries (UtilitiesTest Utilities gtest gtest_main)

add_test(NAME UtilitiesTest COMMAND UtilitiesTest)
