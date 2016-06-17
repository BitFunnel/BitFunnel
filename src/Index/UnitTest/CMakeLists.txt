# BitFunnel/src/Index/UnitTest

set(CPPFILES
    EndToEndTests.cpp
    Placeholder.cpp
    DocumentLengthHistogramUnitTest.cpp
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
include_directories(${CMAKE_SOURCE_DIR}/src/Index/src)


add_executable(IndexUnitTest ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET IndexUnitTest PROPERTY FOLDER "src/Index")
set_property(TARGET IndexUnitTest PROPERTY PROJECT_LABEL "UnitTest")
target_link_libraries (IndexUnitTest CsvTsv Index Utilities gtest gtest_main)

add_test(NAME IndexUnitTest COMMAND IndexUnitTest)
