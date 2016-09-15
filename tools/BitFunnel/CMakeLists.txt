# BitFunnel/tools/BitFunnel

set(CPPFILES
#    main.cpp
    Commands.cpp
    Environment.cpp
#    main.cpp
    REPL.cpp
    StatisticsBuilder.cpp
    TaskFactory.cpp
    TaskPool.cpp
    TermTableBuilder.cpp
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


add_library(BitFunnelTools ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET BitFunnelTools PROPERTY FOLDER "tools")
set_property(TARGET BitFunnelTools PROPERTY PROJECT_LABEL "BitFunnelTools")


add_executable(BitFunnel main.cpp)
target_link_libraries(BitFunnel BitFunnelTools CmdLineParser TestShared Index Configuration CsvTsv Plan Utilities Data)
set_property(TARGET BitFunnel PROPERTY FOLDER "tools")
set_property(TARGET BitFunnel PROPERTY PROJECT_LABEL "BitFunnel")
