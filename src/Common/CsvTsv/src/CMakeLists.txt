# BitFunnel/src/Common/CsvTsv/src

set(CPPFILES
    Csv.cpp
    DelimitedTable.cpp
    ParseError.cpp
    Table.cpp
    Tsv.cpp
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

add_library(CsvTsv ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET CsvTsv PROPERTY FOLDER "src/Common/CsvTsv")
set_property(TARGET CsvTsv PROPERTY PROJECT_LABEL "src")
