# BitFunnel/src/Plan/src

set(CPPFILES
    AbstractRow.cpp
    AbstractRowEnumerator.cpp
    ByteCodeInterpreter.cpp
    CacheLineRecorder.cpp
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
    QueryResources.cpp
    QueryRunner.cpp
    RankDownCompiler.cpp
    RankZeroCompiler.cpp
    RegisterAllocator.cpp
    RowMatchNode.cpp
    RowPlan.cpp
    RowSet.cpp
    StringVector.cpp
    TermMatchNode.cpp
    TermMatchTreeConverter.cpp
    TermMatchTreeEvaluator.cpp
    TermPlan.cpp
    TermPlanConverter.cpp
    VerifyOneQuery.cpp
)

set(WINDOWS_CPPFILES
)

set(POSIX_CPPFILES
)

set(PRIVATE_HFILES
    AbstractRow.h
    ByteCodeInterpreter.h
    CacheLineRecorder.h
    CompileNode.h
    ICodeGenerator.h
    IPlanRows.h
    IRowSet.h
    MachineCodeGenerator.h
    MatchTreeCompiler.h
    MatchTreeRewriter.h
    MatchVerifier.h
    NativeCodeGenerator.h
    QueryPlanner.h
    QueryResources.h
    ResultsBuffer.h
    RowMatchNode.h
    RowSet.h
    RankDownCompiler.h
    RankZeroCompiler.h
    RegisterAllocator.h
    RowPlan.h
    StringVector.h
    TermPlan.h
    TermPlanConverter.h
    TermMatchTreeEvaluator.h
)

set(WINDOWS_PRIVATE_HFILES
)

set(POSIX_PRIVATE_HFILES
)

COMBINE_FILE_LISTS()

add_library(Plan ${CPPFILES} ${PRIVATE_HFILES} ${PUBLIC_HFILES})
set_property(TARGET Plan PROPERTY FOLDER "src/Plan")
set_property(TARGET Plan PROPERTY PROJECT_LABEL "src")
