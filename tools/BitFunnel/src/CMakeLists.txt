# BitFunnel/tools/BitFunnel/src

set(CPPFILES
    BitFunnelTool.cpp
    Commands.cpp
    Environment.cpp
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
    BitFunnelTool.h
    Commands.h
    Environment.h
    ICommand.h
    ITask.h
    REPL.h
    StatisticsBuilder.h
    TaskBase.h
    TaskFactory.h
    TaskPool.h
    TermTableBuilder.h
)

set(WINDOWS_PRIVATE_HFILES
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()


add_library(BitFunnelTool ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET BitFunnelTool PROPERTY FOLDER "tools/BitFunnel")
set_property(TARGET BitFunnelTool PROPERTY PROJECT_LABEL "Library")


add_executable(BitFunnel main.cpp)
target_link_libraries(BitFunnel BitFunnelTool CmdLineParser TestShared Index Configuration CsvTsv Plan Utilities Data)
set_property(TARGET BitFunnel PROPERTY FOLDER "tools/BitFunnel")
set_property(TARGET BitFunnel PROPERTY PROJECT_LABEL "Executable")
