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

#include <algorithm>    // std::sort()
#include <iostream>

#include "BitFunnel/IDiagnosticStream.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/IShard.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Index/ITermTable.h"
#include "BitFunnel/Index/RowIdSequence.h"
#include "BitFunnel/Index/Token.h"
#include "BitFunnel/Plan/Factories.h"
#include "BitFunnel/Plan/IResultsProcessor.h"
#include "BitFunnel/Plan/TermMatchNode.h"
#include "BitFunnel/Term.h"
#include "LoggerInterfaces/Check.h"
#include "SimplePlanner.h"


namespace BitFunnel
{
    std::vector<DocId> Factories::RunSimplePlanner(TermMatchNode const & tree,
                                                   ISimpleIndex const & index,
                                                   IDiagnosticStream& diagnosticStream)
    {
        SimplePlanner simplePlanner(tree, index, diagnosticStream);
        return simplePlanner.GetMatches();
    }


    //*************************************************************************
    //
    // SimplePlanner
    //
    //*************************************************************************
    SimplePlanner::SimplePlanner(TermMatchNode const & tree,
                                 ISimpleIndex const & index,
                                 IDiagnosticStream& diagnosticStream)
        : m_index(index),
          m_resultsProcessor(Factories::CreateSimpleResultsProcessor())

    {
        ExtractRowIds(tree);
        struct
        {
            bool operator() (RowId a, RowId b)
            {
                // Sorts by decreasing rank.
                return a.GetRank() > b.GetRank();
            }
        } compare;

        // Sort RowIds by decreasing Rank.
        std::sort(m_rows.begin(), m_rows.end(), compare);

        CHECK_GT(m_rows.size(), 0u);
        Rank rank = m_rows[0].GetRank();
        m_code.LoadRow(0u, false, 0u);
        Compile(1u, rank);
        m_code.Seal();

        // Get token before we GetSliceBuffers.
        {
            auto token = m_index.GetIngestor().GetTokenManager().RequestToken();

            const size_t c_shardId = 0u;
            auto & shard = m_index.GetIngestor().GetShard(c_shardId);
            auto & sliceBuffers = shard.GetSliceBuffers();
            size_t sliceCount = sliceBuffers.size();

            // Iterations per slice calculation.
            auto iterationsPerSlice = shard.GetSliceCapacity() >> 6 >> rank;

            // Get Row offsets.
            std::vector<ptrdiff_t> rowOffsets;
            for (auto row : m_rows)
            {
                rowOffsets.push_back(shard.GetRowOffset(row));
            }

            ByteCodeInterpreter intepreter(m_code,
                                           *m_resultsProcessor,
                                           sliceCount,
                                           reinterpret_cast<char* const *>(sliceBuffers.data()),
                                           iterationsPerSlice,
                                           rowOffsets.data(),
                                           diagnosticStream);

            intepreter.Run();

        } // End of token lifetime.
    }


    std::vector<DocId> const & SimplePlanner::GetMatches() const
    {
        return m_resultsProcessor->GetMatches();
    }

    //
    // private methods
    //

    void SimplePlanner::Compile(size_t pos, Rank rank)
    {
        if (pos == m_rows.size())
        {
            m_code.Report();
        }
        else
        {
            auto row = m_rows[pos];
            if (row.GetRank() < rank)
            {
                RankDown(pos, rank);
                rank = row.GetRank();
            }
            else
            {
                m_code.AndRow(pos, false, 0);
                Compile(pos + 1, rank);
            }
        }
    }

    void SimplePlanner::RankDown(size_t pos, Rank rank)
    {
        size_t delta = rank - m_rows[pos].GetRank();
        m_code.LeftShiftOffset(delta);
        ICodeGenerator::Label label0 = m_code.AllocateLabel();

        unsigned iterations = (1 << delta) - 1;
        for (unsigned i = 0; i < iterations; ++i)
        {
            m_code.Push();
            m_code.Call(label0);
            m_code.Pop();
            m_code.IncrementOffset();
        }
        m_code.Call(label0);
        ICodeGenerator::Label label1 = m_code.AllocateLabel();
        m_code.Jmp(label1);
        m_code.PlaceLabel(label0);
        Compile(pos, m_rows[pos].GetRank());
        m_code.Return();
        m_code.PlaceLabel(label1);
        m_code.RightShiftOffset(delta);
    }


    void SimplePlanner::ExtractRowIds(TermMatchNode const & node)
    {
        switch (node.GetType())
        {
        case TermMatchNode::AndMatch:
            {
                auto const & andNode = dynamic_cast<const TermMatchNode::And&>(node);
                ExtractRowIds(andNode.GetLeft());
                ExtractRowIds(andNode.GetRight());
            }
            break;
        case TermMatchNode::UnigramMatch:
            {
                auto const & unigramNode = dynamic_cast<const TermMatchNode::Unigram&>(node);
                Term term(unigramNode.GetText(), unigramNode.GetStreamId(), m_index.GetConfiguration());
                RowIdSequence rows(term, m_index.GetTermTable());
                for (auto row : rows)
                {
                    m_rows.push_back(row);
                }
            }
            break;
        default:
            CHECK_FAIL << "Invalid node type.";
        }
    }
}
