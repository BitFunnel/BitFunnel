# BitFunnel/src/Common/CsvTsv

add_subdirectory(src)
add_subdirectory(test)

#
# Create a custom_target for the public header file directory.
#
add_custom_target(CsvTsvInc SOURCES)
set_property(TARGET CsvTsvInc PROPERTY FOLDER "src/Common/CsvTsv")
