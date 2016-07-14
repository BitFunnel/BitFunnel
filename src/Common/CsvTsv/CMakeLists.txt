# BitFunnel/src/Common/CsvTsv

add_subdirectory(src)
add_subdirectory(test)

set(PUBLIC_HFILES
   ${CMAKE_SOURCE_DIR}/src/Common/CsvTsv/inc/CsvTsv/Csv.h
   ${CMAKE_SOURCE_DIR}/src/Common/CsvTsv/inc/CsvTsv/DelimitedTable.h
   ${CMAKE_SOURCE_DIR}/src/Common/CsvTsv/inc/CsvTsv/ParseError.h
   ${CMAKE_SOURCE_DIR}/src/Common/CsvTsv/inc/CsvTsv/Table.h
   ${CMAKE_SOURCE_DIR}/src/Common/CsvTsv/inc/CsvTsv/Tsv.h
)

source_group("CsvTsv" FILES ${PUBLIC_HFILES})

#
# Create a custom_target for the public header file directory.
#
add_custom_target(CsvTsvInc SOURCES ${PUBLIC_HFILES})
set_property(TARGET CsvTsvInc PROPERTY FOLDER "src/Common/CsvTsv")
set_property(TARGET CsvTsvInc PROPERTY PROJECT_LABEL "inc")
