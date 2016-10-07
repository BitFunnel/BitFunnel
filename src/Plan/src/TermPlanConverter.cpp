// The MIT License (MIT)

// Copyright (c) 2016, Microsoft

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <new>  // Required to use placement new.

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
