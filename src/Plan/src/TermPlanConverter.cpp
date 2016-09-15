#include "stdafx.h"

#include "BitFunnelAllocatorInterfaces/IAllocator.h"
#include "PlanRows.h"
#include "BitFunnel/RowPlan.h"
#include "BitFunnel/TermMatchNodes.h"
#include "BitFunnel/TermPlanConverter.h"
#include "TermMatchTreeConverter.h"
#include "TermMatchTreeToFalsePositiveEvaluationTreeConverter.h"

namespace BitFunnel
{
    RowPlan const & TermPlanConverter::BuildRowPlan(TermMatchNode const & termMatchNode, 
                                                    IIndexConfiguration const & index, 
                                                    bool generateNonBodyPlan,
                                                    Allocators::IAllocator& allocator)
    {
        PlanRows& planRows = *new (allocator.Allocate(sizeof(PlanRows))) PlanRows(index);
        TermMatchTreeConverter matchConverter(index, planRows, generateNonBodyPlan, allocator);

        RowMatchNode const & matchTree = matchConverter.BuildRowMatchTree(termMatchNode);

        return *new (allocator.Allocate(sizeof(RowPlan))) RowPlan(matchTree, planRows);
    }


    FalsePositiveEvaluationNode const & TermPlanConverter::BuildFalsePositiveEvaluationPlan(
        const TermMatchNode& termMatchNode,
        Allocators::IAllocator& allocator)
    {
        FalsePositiveEvaluationNode const & falsePositiveTree = 
            TermMatchTreeToFalsePositiveEvaluationTreeConverter::BuildFalsePositiveEvaluationTree(allocator, termMatchNode);

        return falsePositiveTree;
    }
}
