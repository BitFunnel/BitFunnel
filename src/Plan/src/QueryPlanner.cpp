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
#include "BitFunnel/Utilities/Allocator.h"
#include "BitFunnel/Utilities/Factories.h"
#include "BitFunnel/Utilities/IObjectFormatter.h"
#include "ByteCodeInterpreter.h"
#include "CompileNode.h"
#include "IPlanRows.h"
#include "LoggerInterfaces/Logging.h"
#include "MatchTreeCompiler.h"
#include "MatchTreeRewriter.h"
#include "NativeJIT/CodeGen/ExecutionBuffer.h"
#include "QueryPlanner.h"
#include "QueryResources.h"
#include "RankDownCompiler.h"
#include "RegisterAllocator.h"
#include "ResultsBuffer.h"
#include "RowPlan.h"
#include "RowSet.h"
#include "TermPlan.h"
#include "TermPlanConverter.h"


namespace BitFunnel
{
    // TODO: remove. This is a quick shortcut to try to connect QueryPlanner the
    // way SimplePlanner is connected.
    void Factories::RunQueryPlanner(TermMatchNode const & tree,
                                    ISimpleIndex const & index,
                                    QueryResources & resources,
                                    IDiagnosticStream & diagnosticStream,
                                    QueryInstrumentation & instrumentation,
                                    ResultsBuffer & resultsBuffer,
                                    bool useNativeCode)
    {
        const int c_arbitraryRowCount = 500;
        QueryPlanner planner(tree,
                             c_arbitraryRowCount,
                             index,
                             resources,
                             diagnosticStream,
                             instrumentation,
                             resultsBuffer,
                             useNativeCode);
    }


    unsigned const c_targetCrossProductTermCount = 180;

    // TODO: this should take a TermPlan instead of a TermMatchNode when we have
    // scoring and query preferences.
    QueryPlanner::QueryPlanner(TermMatchNode const & tree,
                               unsigned targetRowCount,
                               ISimpleIndex const & index,
                               QueryResources & resources,
                               IDiagnosticStream & diagnosticStream,
                               QueryInstrumentation & instrumentation,
                               ResultsBuffer & resultsBuffer,
                               bool useNativeCode)
      : m_resultsBuffer(resultsBuffer)
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
                                            // generateNonBodyPlan,
                                            resources.GetMatchTreeAllocator());

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
            for (ShardId shard = 0 ; shard < m_planRows->GetShardCount(); ++shard)
            {
                for (unsigned id = 0 ; id < m_planRows->GetRowCount(); ++id)
                {
                    RowId row = m_planRows->PhysicalRow(shard, id);

                    out
                        << "(" << shard << ", " << id << "): "
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
                                       resources.GetMatchTreeAllocator());


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
        RankDownCompiler compiler(resources.GetMatchTreeAllocator());
        compiler.Compile(rewritten);
        const Rank initialRank = compiler.GetMaximumRank();
        CompileNode const & compileTree = compiler.CreateTree(initialRank);

        if (diagnosticStream.IsEnabled("planning/compile"))
        {
            std::ostream& out = diagnosticStream.GetStream();
            std::unique_ptr<IObjectFormatter>
                formatter(Factories::CreateObjectFormatter(diagnosticStream.GetStream()));

            out << "--------------------" << std::endl;
            out << "Compile Nodes:" << std::endl;
            compileTree.Format(*formatter);
            out << std::endl;
        }

        RowSet rowSet(index, *m_planRows, resources.GetMatchTreeAllocator());
        rowSet.LoadRows();

        if (diagnosticStream.IsEnabled("planning/rowset"))
        {
            std::ostream& out = diagnosticStream.GetStream();
            out << "--------------------" << std::endl;
            out << "Row Set:" << std::endl;
            out << "  ShardCount: " << rowSet.GetShardCount() << std::endl;
            out << "  Row Count: " << rowSet.GetRowCount() << std::endl;

        }

        instrumentation.SetRowCount(rowSet.GetRowCount());

        if (useNativeCode)
        {
            RunNativeCode(index,
                          resources,
                          instrumentation,
                          compileTree,
                          initialRank,
                          rowSet);
        }
        else
        {
            RunByteCodeInterpreter(index,
                                   resources,
                                   instrumentation,
                                   compileTree,
                                   initialRank,
                                   rowSet);
        }
    }


    void QueryPlanner::RunByteCodeInterpreter(ISimpleIndex const & index,
                                              QueryResources & resources,
                                              QueryInstrumentation & instrumentation,
                                              CompileNode const & compileTree,
                                              Rank initialRank,
                                              RowSet const & rowSet)
    {
        // TODO: Clear results buffer here?
        compileTree.Compile(m_code);
        m_code.Seal();

        instrumentation.FinishPlanning();

        // Get token before we GetSliceBuffers.
        {
            auto token = index.GetIngestor().GetTokenManager().RequestToken();

            for (ShardId shardId = 0; shardId < index.GetIngestor().GetShardCount(); ++shardId)
            {
                auto & shard = index.GetIngestor().GetShard(shardId);
                auto & sliceBuffers = shard.GetSliceBuffers();

                // Iterations per slice calculation.
                auto iterationsPerSlice = shard.GetSliceCapacity() >> 6 >> initialRank;

                m_resultsBuffer.Reset();

                ByteCodeInterpreter intepreter(m_code,
                                               m_resultsBuffer,
                                               sliceBuffers.size(),
                                               sliceBuffers.data(),
                                               iterationsPerSlice,
                                               initialRank,
                                               rowSet.GetRowOffsets(shardId),
                                               nullptr,
                                               instrumentation,
                                               resources.GetCacheLineRecorder());

                intepreter.Run();
            }

            instrumentation.FinishMatching();
            instrumentation.SetMatchCount(m_resultsBuffer.size());
        } // End of token lifetime.
    }


    void QueryPlanner::RunNativeCode(ISimpleIndex const & index,
                                     QueryResources & resources,
                                     QueryInstrumentation & instrumentation,
                                     CompileNode const & compileTree,
                                     Rank initialRank,
                                     RowSet const & rowSet)
    {
         // Perform register allocation on the compile tree.
         RegisterAllocator const registers(compileTree,
                                           rowSet.GetRowCount(),
                                           c_registerBase,
                                           c_registerCount,
                                           resources.GetMatchTreeAllocator());

         MatchTreeCompiler compiler(resources,
                                    compileTree,
                                    registers,
                                    initialRank);


         // TODO: Clear results buffer here?
        compileTree.Compile(m_code);
        m_code.Seal();

        instrumentation.FinishPlanning();

        // Get token before we GetSliceBuffers.
        {
            auto token = index.GetIngestor().GetTokenManager().RequestToken();

            for (ShardId shardId = 0; shardId < index.GetIngestor().GetShardCount(); ++shardId)
            {
                auto & shard = index.GetIngestor().GetShard(shardId);
                auto & sliceBuffers = shard.GetSliceBuffers();

                // Iterations per slice calculation.
                auto iterationsPerSlice = shard.GetSliceCapacity() >> 6 >> initialRank;


                m_resultsBuffer.Reset();

                size_t quadwordCount = compiler.Run(sliceBuffers.size(),
                                                    sliceBuffers.data(),
                                                    iterationsPerSlice,
                                                    rowSet.GetRowOffsets(shardId),
                                                    m_resultsBuffer);

                instrumentation.IncrementQuadwordCount(quadwordCount);
            }

            instrumentation.FinishMatching();
            instrumentation.SetMatchCount(m_resultsBuffer.size());
        } // End of token lifetime.
    }


    IPlanRows const & QueryPlanner::GetPlanRows() const
    {
        return *m_planRows;
    }
}
