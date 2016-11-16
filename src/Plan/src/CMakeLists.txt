# BitFunnel/src/Plan/src

set(CPPFILES
    AbstractRow.cpp
    AbstractRowEnumerator.cpp
    ByteCodeInterpreter.cpp
    CompileNode.cpp
    MachineCodeGenerator.cpp
    MatchTreeCompiler.cpp
    MatchTreeRewriter.cpp
    MatchVerifier.cpp
    NativeCodeGenerator.cpp
    PlanRows.cpp
    QueryInstrumentation.cpp
    QueryParser.cpp
    QueryPlanner.cpp
    QueryRunner.cpp
    RankDownCompiler.cpp
    RankZeroCompiler.cpp
    RegisterAllocator.cpp
    RowMatchNode.cpp
    RowPlan.cpp
    RowSet.cpp
    SimpleResultsProcessor.cpp
    StringVector.cpp
    TermMatchNode.cpp
    TermMatchTreeConverter.cpp
    TermMatchTreeEvaluator.cpp
    TermPlan.cpp
    TermPlanConverter.cpp
)

set(WINDOWS_CPPFILES
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
    ByteCodeInterpreter.h
    CompileNode.h
    MachineCodeGenerator.h
    MatchTreeCompiler.h
    MatchTreeRewriter.h
    MatchVerifier.h
    NativeCodeGenerator.h
    QueryPlanner.h
    RowSet.h
    RankDownCompiler.h
    RankZeroCompiler.h
    RegisterAllocator.h
    StringVector.h
)

set(WINDOWS_PRIVATE_HFILES
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()

add_library(Plan ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET Plan PROPERTY FOLDER "src/Plan")
set_property(TARGET Plan PROPERTY PROJECT_LABEL "src")
