# BitFunnel/src/Plan/src

set(CPPFILES
    AbstractRow.cpp
    AbstractRowEnumerator.cpp
    ByteCodeInterpreter.cpp
    CompileNode.cpp
    MatchTreeRewriter.cpp
    PlanRows.cpp
    QueryParser.cpp
    QueryPipeline.cpp
    QueryPlanner.cpp
    RankDownCompiler.cpp
    RankZeroCompiler.cpp
    RegisterAllocator.cpp
    RowMatchNode.cpp
    RowPlan.cpp
    SimplePlanner.cpp
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
    MatchTreeRewriter.h
    RankDownCompiler.h
    RankZeroCompiler.h
    RegisterAllocator.h
    SimplePlanner.h
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
