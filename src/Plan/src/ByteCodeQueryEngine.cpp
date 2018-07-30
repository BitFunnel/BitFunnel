// The MIT License (MIT)

// Copyright (c) 2018, Microsoft

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

#include <iostream>

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Plan/Factories.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/IShard.h"
#include "BitFunnel/Index/Token.h"
#include "BitFunnel/Plan/QueryInstrumentation.h"
#include "BitFunnel/Plan/QueryParser.h"
#include "BitFunnel/Plan/ResultsBuffer.h"
#include "BitFunnel/Utilities/Allocator.h"
#include "BitFunnel/Utilities/Factories.h"
#include "ByteCodeQueryEngine.h"
#include "CompileNode.h"
#include "QueryPlanner.h"
#include "RowSet.h"


namespace BitFunnel
{
    ByteCodeQueryEngine::ByteCodeQueryEngine(ISimpleIndex const & index,
                                             IStreamConfiguration const & config,
                                             size_t treeAllocatorBytes)
        : m_index(index),
          m_config(config),
          m_diagnostic(Factories::CreateDiagnosticStream(std::cout)),
          m_matchTreeAllocator(new BitFunnel::Allocator(treeAllocatorBytes))
    {
    }

    // Parse a query
    TermMatchNode const *ByteCodeQueryEngine::Parse(const char *query)
    {
        m_matchTreeAllocator->Reset();
        QueryParser parser(query,
            m_config,
            *m_matchTreeAllocator);
        return parser.Parse();
    }

    // Runs a parsed query
    void ByteCodeQueryEngine::Run(TermMatchNode const * tree,
        QueryInstrumentation & instrumentation,
        ResultsBuffer & resultsBuffer)
    {
        const int c_arbitraryRowCount = 500;
        QueryPlanner planner(*tree,
                             c_arbitraryRowCount,
                             m_index,
                             *m_matchTreeAllocator,
                             *m_diagnostic,
                             instrumentation);
        CompileNode const & compileTree = planner.GetCompileTree();
        const Rank initialRank = planner.GetInitialRank();
        const RowSet & rowSet = planner.GetRowSet();

        // TODO: Clear results buffer here?
        compileTree.Compile(m_code);
        m_code.Seal();

        instrumentation.FinishPlanning();
        resultsBuffer.Reset();

        // Get token before we GetSliceBuffers.
        {
            auto token = m_index.GetIngestor().GetTokenManager().RequestToken();

            for (ShardId shardId = 0; shardId < m_index.GetIngestor().GetShardCount(); ++shardId)
            {
                auto & shard = m_index.GetIngestor().GetShard(shardId);
                auto & sliceBuffers = shard.GetSliceBuffers();

                // Iterations per slice calculation.
                auto iterationsPerSlice = shard.GetSliceCapacity() >> 6 >> initialRank;

                auto countCacheLines = m_diagnostic->IsEnabled("planning/countcachelines");

                ByteCodeInterpreter interpreter(m_code,
                    resultsBuffer,
                    sliceBuffers.size(),
                    sliceBuffers.data(),
                    iterationsPerSlice,
                    initialRank,
                    rowSet.GetRowOffsets(shardId),
                    nullptr,
                    instrumentation,
                    countCacheLines ? shard.GetSliceBufferSize() : 0);

                interpreter.Run();
            }

            instrumentation.FinishMatching();
            instrumentation.SetMatchCount(resultsBuffer.size());
            instrumentation.QuerySucceeded();
        } // End of token lifetime.
    }


    // Adds the diagnostic keyword prefix to the list of prefixes that
    // enable diagnostics.
    void ByteCodeQueryEngine::EnableDiagnostic(char const * prefix)
    {
        m_diagnostic->Enable(prefix);
    }

    // Removes the diagnostic keyword prefix from the list of prefixes
    // that enable diagnostics.
    void ByteCodeQueryEngine::DisableDiagnostic(char const * prefix)
    {
        m_diagnostic->Disable(prefix);
    }

}