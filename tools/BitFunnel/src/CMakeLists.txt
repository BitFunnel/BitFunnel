# BitFunnel/tools/BitFunnel/src

set(CPPFILES
    AnalyzeCommand.cpp
    BitFunnelTool.cpp
    CdCommand.cpp
    Commands.cpp
    Environment.cpp
    ExitCommand.cpp
    FailOnExceptionCommand.cpp
    HelpCommand.cpp
    IngestCommands.cpp
    QueryCommand.cpp
    REPL.cpp
    ScriptCommand.cpp
    ShowCommand.cpp
    StatisticsBuilder.cpp
    TaskFactory.cpp
    TaskPool.cpp
    TermTableBuilderTool.cpp
)

set(WINDOWS_CPPFILES
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
    AnalyzeCommand.h
    BitFunnelTool.h
    CdCommand.h
    Commands.h
    ExitCommand.h
    FailOnExceptionCommand.h
    Environment.h
    HelpCommand.h
    IngestCommands.h
    ICommand.h
    ITask.h
    ScriptCommand.h
    ShowCommand.h
    TaskBase.h
    TaskFactory.h
    QueryCommand.h
    REPL.h
    StatisticsBuilder.h
    TaskPool.h
    TermTableBuilderTool.h
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


add_executable(TheBard TheBard.cpp)
target_link_libraries(TheBard BitFunnelTool CmdLineParser TestShared Index Configuration CsvTsv Plan Utilities Data)
set_property(TARGET TheBard PROPERTY FOLDER "tools/TheBard")
set_property(TARGET TheBard PROPERTY PROJECT_LABEL "Executable")
