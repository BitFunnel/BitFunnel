# BitFunnel/tools/IngestAndQuery

set(CPPFILES
    main.cpp
    Commands.cpp
    Environment.cpp
    REPL.cpp
    TaskFactory.cpp
    TaskPool.cpp
)

set(WINDOWS_CPPFILES
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
    Commands.h
    Environment.h
    ICommand.h
    ITask.h
    REPL.h
    TaskBase.h
    TaskFactory.h
    TaskPool.h
)

set(WINDOWS_PRIVATE_HFILES
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()


add_executable(IngestAndQuery ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
target_link_libraries(IngestAndQuery CmdLineParser TestShared Index Configuration CsvTsv Utilities Plan)
set_property(TARGET IngestAndQuery PROPERTY FOLDER "tools")
set_property(TARGET IngestAndQuery PROPERTY PROJECT_LABEL "IngestAndQuery")
