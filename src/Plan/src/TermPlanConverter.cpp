#include <new>

#include "BitFunnel/Allocators/IAllocator.h"
#include "PlanRows.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Plan/RowPlan.h"
#include "BitFunnel/Plan/TermMatchNode.h"
#include "BitFunnel/Plan/TermPlanConverter.h"
#include "TermMatchTreeConverter.h"
// #include "TermMatchTreeToFalsePositiveEvaluationTreeConverter.h"

namespace BitFunnel
{
    RowPlan const & TermPlanConverter::BuildRowPlan(TermMatchNode const & termMatchNode,
                                                    ISimpleIndex const & index,
                                                    // bool generateNonBodyPlan,
                                                    IAllocator& allocator)
    {
        // TODO: will need to modify this if we restore something like
        // generateNonBodyPlan.
        PlanRows& planRows = *new (allocator.Allocate(sizeof(PlanRows)))
            PlanRows(index);
        // TermMatchTreeConverter matchConverter(index, planRows, generateNonBodyPlan, allocator);
        TermMatchTreeConverter matchConverter(index, planRows, allocator);

        RowMatchNode const & matchTree = matchConverter.
            BuildRowMatchTree(termMatchNode);

        return *new (allocator.Allocate(sizeof(RowPlan)))
            RowPlan(matchTree, planRows);
    }


    // FalsePositiveEvaluationNode const & TermPlanConverter::BuildFalsePositiveEvaluationPlan(
    //     const TermMatchNode& termMatchNode,
    //     IAllocator& allocator)
    // {
    //     FalsePositiveEvaluationNode const & falsePositiveTree =
    //         TermMatchTreeToFalsePositiveEvaluationTreeConverter::BuildFalsePositiveEvaluationTree(allocator, termMatchNode);

    //     return falsePositiveTree;
    // }
}
