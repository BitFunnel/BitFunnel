# BitFunnel/src/Common/CsvTsv/UnitTest

set(CPPFILES
)

set(WINDOWS_CPPFILES
    DelimitedTableUnitTest.cpp
    ReadWriteSingleColumnUnitTest.cpp
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

add_executable(CsvTsvUnitTest ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET CsvTsvUnitTest PROPERTY FOLDER "src/Common/CsvTsv")
set_property(TARGET CsvTsvUnitTest PROPERTY PROJECT_LABEL "UnitTest")
target_link_libraries (CsvTsvUnitTest CsvTsv gtest gtest_main)

add_test(NAME CsvTsvUnitTest COMMAND CsvTsvUnitTest)
