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

#include "Allocator.h"
#include "BitFunnel/Allocators/IAllocator.h"
// #include "BitFunnel/CompiledFunction.h"
#include "BitFunnel/IDiagnosticStream.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Index/IShard.h"
#include "BitFunnel/Index/Token.h"
#include "BitFunnel/Plan/Factories.h"
#include "BitFunnel/Plan/IPlanRows.h"
// #include "BitFunnel/IThreadResources.h"
#include "BitFunnel/Plan/RowPlan.h"
#include "BitFunnel/Plan/TermMatchNode.h"
#include "BitFunnel/Plan/TermPlan.h"
#include "BitFunnel/Plan/TermPlanConverter.h"
#include "BitFunnel/Utilities/Factories.h"
#include "BitFunnel/Utilities/IObjectFormatter.h"
#include "ByteCodeInterpreter.h"
#include "CompileNode.h"
#include "LoggerInterfaces/Logging.h"
// #include "MatchTreeCodeGenerator.h"
#include "MatchTreeRewriter.h"
#include "QueryPlanner.h"
#include "RankDownCompiler.h"
#include "RegisterAllocator.h"
#include "RowSet.h"


namespace BitFunnel
{
    // TODO: remove. This is a quick shortcut to try to connect QueryPlanner the
    // way SimplePlanner is connected.
    std::vector<DocId> Factories::RunQueryPlanner(TermMatchNode const & tree,
                                                  ISimpleIndex const & index,
                                                  IDiagnosticStream& diagnosticStream)
    {
        // TODO: this really shouldn't create its own allocator.
        Allocator allocator(4096*16);
        const int c_arbitraryRowCount = 500;
        QueryPlanner planner(tree, c_arbitraryRowCount, index, allocator, diagnosticStream);
        return planner.GetMatches();
    }


    // QueryPlanner::X64FunctionGeneratorWrapper::X64FunctionGeneratorWrapper(IThreadResources& threadResources)
    //     : m_code(threadResources.AllocateFunctionGenerator()),
    //       m_threadResources(threadResources)
    // {
    // }


    // QueryPlanner::X64FunctionGeneratorWrapper::~X64FunctionGeneratorWrapper()
    // {
    //     m_threadResources.ReleaseFunctionGenerator(m_code);
    // }


    // QueryPlanner::X64FunctionGeneratorWrapper::operator X64::X64FunctionGenerator&() const
    // {
    //     return m_code;
    // }


    unsigned const c_targetCrossProductTermCount = 180;

    // TODO: this should take a TermPlan instead of a TermMatchNode when we have
    // scoring and query preferences.
    QueryPlanner::QueryPlanner(TermMatchNode const & tree,
                               unsigned targetRowCount,
                               ISimpleIndex const & index,
                               // IThreadResources& threadResources,
                               IAllocator& allocator,
                               IDiagnosticStream& diagnosticStream)
                               // bool generateNonBodyPlan,
                               // unsigned maxIterationsScannedBetweenTerminationChecks)
        : m_resultsProcessor(Factories::CreateSimpleResultsProcessor())
    // : // m_x64FunctionGeneratorWrapper(threadResources),
    //   m_maxIterationsScannedBetweenTerminationChecks(maxIterationsScannedBetweenTerminationChecks)
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
                                            allocator);

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
                                       allocator);


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
        RankDownCompiler compiler(allocator);
        compiler.Compile(rewritten);
        CompileNode const & compileTree = compiler.CreateTree(c_maxRankValue);

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

        // This has been ported, but doesn't seem necessary for the
        // ByteCodeInterpreter?
        //
        // // Perform register allocation on the compile tree.
        // RegisterAllocator const registers(compileTree,
        //                                   rowPlan.GetPlanRows().GetRowCount(),
        //                                   c_registerBase,
        //                                   c_registerCount,
        //                                   allocator);

        compileTree.Compile(m_code);
        m_code.Seal();

        // Finally, translate compile tree to X64 machine code.
        // MatchTreeCodeGenerator generator(registers,
        //                                  m_x64FunctionGeneratorWrapper,
        //                                  m_maxIterationsScannedBetweenTerminationChecks);
        // generator.GenerateX64Code(compileTree);

        RowSet rowSet(index, *m_planRows, allocator);
        rowSet.LoadRows();

        // TODO: Move all of this somewhere else.
        // Get token before we GetSliceBuffers.
        {
            auto token = index.GetIngestor().GetTokenManager().RequestToken();

            const size_t c_shardId = 0u;
            auto & shard = index.GetIngestor().GetShard(c_shardId);
            auto & sliceBuffers = shard.GetSliceBuffers();
            size_t sliceCount = sliceBuffers.size();

            // Iterations per slice calculation.
            const int c_horribleRankHack = 3;
            auto iterationsPerSlice = shard.GetSliceCapacity() >> 6 >> c_horribleRankHack;

            ByteCodeInterpreter intepreter(m_code,
                                           *m_resultsProcessor,
                                           sliceCount,
                                           reinterpret_cast<char* const *>(sliceBuffers.data()),
                                           iterationsPerSlice,
                                           rowSet.GetRowOffsets(c_shardId),
                                           diagnosticStream);

            intepreter.Run();

        } // End of token lifetime.
    }


//     const CompiledFunction QueryPlanner::GetMatchingFunction() const
// OA    {
//         // TODO: Don't like how call to GetMatchingFunction() finalizes jumps. This should be explicit.
//         return CompiledFunction((X64::X64FunctionGenerator&)m_x64FunctionGeneratorWrapper);
//     }

    std::vector<DocId> const & QueryPlanner::GetMatches() const
    {
        return m_resultsProcessor->GetMatches();
    }


    IPlanRows const & QueryPlanner::GetPlanRows() const
    {
        return *m_planRows;
    }
}
