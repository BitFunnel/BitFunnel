#include <algorithm>    // std::sort()
#include <iostream>

#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/IShard.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Index/ITermTable.h"
#include "BitFunnel/Index/RowIdSequence.h"
#include "BitFunnel/Plan/Factories.h"
#include "BitFunnel/Plan/IResultsProcessor.h"
#include "BitFunnel/Plan/TermMatchNode.h"
#include "BitFunnel/Term.h"
#include "LoggerInterfaces/Check.h"
#include "SimplePlanner.h"

namespace BitFunnel
{

    void SimplePlanner::AddResult(uint64_t accumulator,
                                  size_t offset)
    {
        std::cout << "AddResult acc:offset " << std::hex << accumulator
                  << std::dec << ":" << offset << std::endl;
    }


    bool SimplePlanner::FinishIteration(void const * sliceBuffer)
    {
        std::cout << "FinishIteration " << std::hex << sliceBuffer << std::dec << std::endl;
        return false;
    }


    bool SimplePlanner::TerminatedEarly() const
    {
        return false;
    }

    void Factories::RunSimplePlanner(TermMatchNode const & tree, ISimpleIndex const & index)
    {
        SimplePlanner simplePlanner(tree, index);
    }


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


    SimplePlanner::SimplePlanner(TermMatchNode const & tree, ISimpleIndex const & index)
        : m_index(index)

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
        for (auto row : m_rows)
        {
            std::cout << "rank:index " << row.GetRank() << ":" << row.GetIndex() << std::endl;
        }

        CHECK_GT(m_rows.size(), 0u);
        Rank rank = m_rows[0].GetRank();
        // false:0 is inverted:rankDelta.
        m_code.LoadRow(0u, false, 0u);
        Compile(1u, rank);
        m_code.Seal();

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
                                       *this,
                                       sliceCount,
                                       reinterpret_cast<char* const *>(sliceBuffers.data()),
                                       iterationsPerSlice,
                                       rowOffsets.data());

        intepreter.Run();
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
