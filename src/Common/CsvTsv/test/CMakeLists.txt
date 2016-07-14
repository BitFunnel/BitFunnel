# BitFunnel/src/Common/CsvTsv/test

set(CPPFILES
    DelimitedTableUnitTest.cpp
    ReadWriteSingleColumnUnitTest.cpp
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

add_executable(CsvTsvTest ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET CsvTsvTest PROPERTY FOLDER "src/Common/CsvTsv")
set_property(TARGET CsvTsvTest PROPERTY PROJECT_LABEL "Test")
target_link_libraries (CsvTsvTest CsvTsv gtest gtest_main)

add_test(NAME CsvTsvTest COMMAND CsvTsvTest)
