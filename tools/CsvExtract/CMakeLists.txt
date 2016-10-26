# BitFunnel/tools/CsvExtract

set(CPPFILES
    main.cpp
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


add_executable(CsvExtract main.cpp)
target_link_libraries(CsvExtract CmdLineParser CsvTsv)
set_property(TARGET CsvExtract PROPERTY FOLDER "tools")
set_property(TARGET CsvExtract PROPERTY PROJECT_LABEL "CsvExtract")
