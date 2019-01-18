# BitFunnel/tools/BitFunnel/src

set(CPPFILES
    AnalyzeCommand.cpp
    BitFunnelTool.cpp
    CacheLineCountCommand.cpp
    CdCommand.cpp
    CompilerCommand.cpp
    CorrelateCommand.cpp
    Environment.cpp
    ExitCommand.cpp
    FailOnExceptionCommand.cpp
    FilterChunks.cpp
    HelpCommand.cpp
    IngestCommands.cpp
    InterpreterCommand.cpp
    QueryCommand.cpp
    QueryGenerator.cpp
    QueryLogBuilderTool.cpp
    REPL.cpp
    SaveCommand.cpp
    ScriptCommand.cpp
    ShardBuilder.cpp
    ShardCommand.cpp
    ShowCommand.cpp
    StatisticsBuilder.cpp
    StatusCommand.cpp
    TaskFactory.cpp
    TaskPool.cpp
    TermTableBuilderTool.cpp
    ThreadsCommand.cpp
    VerifyCommand.cpp
)

set(WINDOWS_CPPFILES
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
    AnalyzeCommand.h
    BitFunnelTool.h
    CacheLineCountCommand.h
    CdCommand.h
    CompilerCommand.h
    CorrelateCommand.h
    ExitCommand.h
    FailOnExceptionCommand.h
    FilterChunks.h
    Environment.h
    HelpCommand.h
    IngestCommands.h
    ICommand.h
    InterpreterCommand.h
    ITask.h
    QueryCommand.h
    QueryGenerator.h
    QueryLogBuilderTool.h
    REPL.h
    SaveCommand.h
    ScriptCommand.h
    ShardBuilder.h
    ShardCommand.h
    ShowCommand.h
    StatisticsBuilder.h
    StatusCommand.h
    TaskBase.h
    TaskPool.h
    TaskFactory.h
    TermTableBuilderTool.h
    ThreadsCommand.h
    VerifyCommand.h
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
target_link_libraries(BitFunnel BitFunnelTool CmdLineParser TestShared Index Chunks Plan Configuration CsvTsv Utilities Data NativeJIT CodeGen)
set_property(TARGET BitFunnel PROPERTY FOLDER "tools/BitFunnel")
set_property(TARGET BitFunnel PROPERTY PROJECT_LABEL "Executable")


add_executable(TheBard TheBard.cpp)
target_link_libraries(TheBard BitFunnelTool CmdLineParser TestShared Index Chunks Plan Configuration CsvTsv Utilities Data NativeJIT CodeGen)
set_property(TARGET TheBard PROPERTY FOLDER "tools/TheBard")
set_property(TARGET TheBard PROPERTY PROJECT_LABEL "Executable")
