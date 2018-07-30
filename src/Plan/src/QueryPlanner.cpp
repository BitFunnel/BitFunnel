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

#include "BitFunnel/Allocators/IAllocator.h"
#include "BitFunnel/IDiagnosticStream.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Index/IShard.h"
#include "BitFunnel/Index/Token.h"
#include "BitFunnel/Plan/Factories.h"
#include "BitFunnel/Plan/QueryInstrumentation.h"
#include "BitFunnel/Plan/TermMatchNode.h"
#include "BitFunnel/Utilities/Factories.h"
#include "BitFunnel/Utilities/IObjectFormatter.h"
#include "CompileNode.h"
#include "IPlanRows.h"
#include "MatchTreeRewriter.h"
#include "QueryPlanner.h"
#include "RankDownCompiler.h"
#include "RowSet.h"
#include "TermPlan.h"
#include "TermPlanConverter.h"


namespace BitFunnel
{

    unsigned const c_targetCrossProductTermCount = 180;

    // TODO: this should take a TermPlan instead of a TermMatchNode when we have
    // scoring and query preferences.
    QueryPlanner::QueryPlanner(TermMatchNode const & tree,
                               unsigned targetRowCount,
                               ISimpleIndex const & index,
                               IAllocator & matchTreeAllocator,
                               IDiagnosticStream & diagnosticStream,
                               QueryInstrumentation & instrumentation)
    {
        if (diagnosticStream.IsEnabled("planning/term"))
        {
            std::ostream& out = diagnosticStream.GetStream();
            std::unique_ptr<IObjectFormatter>
                formatter(Factories::CreateObjectFormatter(diagnosticStream.GetStream()));

            out << "--------------------" << std::endl;
            out << "Term Plan:" << std::endl;
            tree.Format(*formatter);
            out << std::endl;
        }

        RowPlan const & rowPlan =
            TermPlanConverter::BuildRowPlan(tree,
                                            index,
                                            matchTreeAllocator);

        if (diagnosticStream.IsEnabled("planning/row"))
        {
            std::ostream& out = diagnosticStream.GetStream();
            std::unique_ptr<IObjectFormatter>
                formatter(Factories::CreateObjectFormatter(diagnosticStream.GetStream()));

            out << "--------------------" << std::endl;
            out << "Row Plan:" << std::endl;
            rowPlan.Format(*formatter);
            out << std::endl;
        }

        m_planRows = &rowPlan.GetPlanRows();

        if (diagnosticStream.IsEnabled("planning/planrows"))
        {
            std::ostream& out = diagnosticStream.GetStream();
            std::unique_ptr<IObjectFormatter>
                formatter(Factories::CreateObjectFormatter(diagnosticStream.GetStream()));

            out << "--------------------" << std::endl;
            out << "IPlanRows:" << std::endl;
            out << "  ShardCount: " << m_planRows->GetShardCount() << std::endl;
            for (ShardId shard = 0 ; shard < m_planRows->GetShardCount(); ++shard)
            {
                for (unsigned id = 0 ; id < m_planRows->GetRowCount(); ++id)
                {
                    RowId row = m_planRows->PhysicalRow(shard, id);

                    out
                        << "  (" << shard << ", " << id << "): "
                        << "RowId(" << ", " << row.GetRank()
                        << ", " << row.GetIndex() << ")" << std::endl;
                }
            }
        }

        // Rewrite match tree to optimal form for the RankDownCompiler.
        RowMatchNode const & rewritten =
            MatchTreeRewriter::Rewrite(rowPlan.GetMatchTree(),
                                       targetRowCount,
                                       c_targetCrossProductTermCount,
                                       matchTreeAllocator);


        if (diagnosticStream.IsEnabled("planning/rewrite"))
        {
            std::ostream& out = diagnosticStream.GetStream();
            std::unique_ptr<IObjectFormatter>
                formatter(Factories::CreateObjectFormatter(diagnosticStream.GetStream()));

            out << "--------------------" << std::endl;
            out << "Rewritten Plan:" << std::endl;
            rewritten.Format(*formatter);
            out << std::endl;
        }

        // Compile the match tree into CompileNodes.
        RankDownCompiler compiler(matchTreeAllocator);
        compiler.Compile(rewritten);
        m_initialRank = compiler.GetMaximumRank();
        m_compileTree = &compiler.CreateTree(m_initialRank);

        if (diagnosticStream.IsEnabled("planning/compile"))
        {
            std::ostream& out = diagnosticStream.GetStream();
            std::unique_ptr<IObjectFormatter>
                formatter(Factories::CreateObjectFormatter(diagnosticStream.GetStream()));

            out << "--------------------" << std::endl;
            out << "Compile Nodes:" << std::endl;
            m_compileTree->Format(*formatter);
            out << std::endl;
        }

        m_rowSet = std::unique_ptr<RowSet>(new RowSet(index, *m_planRows, matchTreeAllocator));
        m_rowSet->LoadRows();

        if (diagnosticStream.IsEnabled("planning/rowset"))
        {
            std::ostream& out = diagnosticStream.GetStream();
            out << "--------------------" << std::endl;
            out << "Row Set:" << std::endl;
            out << "  ShardCount: " << m_rowSet->GetShardCount() << std::endl;
            out << "  Row Count: " << m_rowSet->GetRowCount() << std::endl;

        }

        instrumentation.SetRowCount(m_rowSet->GetRowCount());

    }


    CompileNode const & QueryPlanner::GetCompileTree() const
    {
        return *m_compileTree;
    }

    Rank QueryPlanner::GetInitialRank() const
    {
        return m_initialRank;
    }

    RowSet const & QueryPlanner::GetRowSet() const
    {
        return *m_rowSet;
    }

    IPlanRows const & QueryPlanner::GetPlanRows() const
    {
        return *m_planRows;
    }
}
