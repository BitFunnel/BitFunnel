# BitFunnel/src/Common/Utilities/UnitTest

set(CPPFILES
    MurmurHashUnitTest.cpp
    TaskDistributorUnitTest.cpp
    TokenTrackerUnitTest.cpp
    TokenUnitTest.cpp
)

set(WINDOWS_CPPFILES
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
)

set(WINDOWS_PRIVATE_HFILES
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()

# Unit tests are allowed to access private headers of the library they test.
include_directories(${CMAKE_SOURCE_DIR}/src/Common/Utilities/src)


add_executable(UtilitiesUnitTest ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET UtilitiesUnitTest PROPERTY FOLDER "src/Common/Utilities")
set_property(TARGET UtilitiesUnitTest PROPERTY PROJECT_LABEL "UnitTest")
target_link_libraries (UtilitiesUnitTest Utilities gtest gtest_main)

add_test(NAME UtilitiesUnitTest COMMAND UtilitiesUnitTest)
