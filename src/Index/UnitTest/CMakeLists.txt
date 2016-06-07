# BitFunnel/src/Index/UnitTest

set(CPPFILES
)

set(WINDOWS_CPPFILES
    Placeholder.cpp
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
# include_directories(${CMAKE_SOURCE_DIR}/src/Common/Utilities/src)


add_executable(IndexUnitTest ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET IndexUnitTest PROPERTY FOLDER "src/Index")
set_property(TARGET IndexUnitTest PROPERTY PROJECT_LABEL "UnitTest")
target_link_libraries (IndexUnitTest Utilities gtest gtest_main)

add_test(NAME IndexUnitTest COMMAND UtilitiesUnitTest)
