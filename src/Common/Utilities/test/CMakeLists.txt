# BitFunnel/src/Common/Utilities/test

# TODO: move ThrowingLogger to some shared folder?

set(CPPFILES
    Array2DFixedTest.cpp
    Array3DFixedTest.cpp
    Array2DTest.cpp
    BlockAllocatorTest.cpp
    BlockingQueueTest.cpp
    ConstructorDestructorCounter.cpp
    FileHeaderTest.cpp
    MurmurHashTest.cpp
    PackedArrayTest.cpp
    RandomTest.cpp
    RoundingTest.cpp
    SimpleHashSetTest.cpp
    SimpleHashTableTest.cpp
    StreamUtilitiesTest.cpp
    TaskDistributorTest.cpp
    # TermTableTest.cpp
    ThrowingLogger.cpp
    TokenManagerTest.cpp
    TokenTrackerTest.cpp
    TokenTest.cpp
    VersionTest.cpp
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
