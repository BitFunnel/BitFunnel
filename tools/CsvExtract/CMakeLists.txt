# BitFunnel/tools/CsvExtract

set(CPPFILES
    CsvExtract.cpp
)

set(PRIVATE_HFILES
    CsvExtract.h
)


add_executable(CsvExtract ${CPPFILES} ${PRIVATE_HFILES} main.cpp)
target_link_libraries(CsvExtract CmdLineParser CsvTsv Plan Utilities)
set_property(TARGET CsvExtract PROPERTY FOLDER "tools/CsvExtract")
set_property(TARGET CsvExtract PROPERTY PROJECT_LABEL "Executable")

add_executable(CsvExtractTest ${CPPFILES} ${PRIVATE_HFILES} CsvExtractTest.cpp)
target_link_libraries(CsvExtractTest CmdLineParser CsvTsv Plan Utilities gtest gtest_main)
set_property(TARGET CsvExtractTest PROPERTY FOLDER "tools/CsvExtract")
set_property(TARGET CsvExtractTest PROPERTY PROJECT_LABEL "Test")
add_test(NAME CsvExtractTest COMMAND CsvExtractTest)
