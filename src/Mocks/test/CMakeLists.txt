# BitFunnel/src/Mocks/test

set(CPPFILES
    PrimeFactorsDocumentTest.cpp
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
include_directories(${CMAKE_SOURCE_DIR}/src/Mocks/src)

add_executable(MocksTest ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET MocksTest PROPERTY FOLDER "src/Mocks")
set_property(TARGET MocksTest PROPERTY PROJECT_LABEL "Test")

target_link_libraries (MocksTest Mocks gtest gtest_main)

add_test(NAME MocksTest COMMAND MocksTest)
