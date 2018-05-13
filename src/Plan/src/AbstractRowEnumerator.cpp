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


#include "AbstractRowEnumerator.h"
#include "BitFunnel/Index/ITermTable.h"
#include "BitFunnel/Index/RowIdSequence.h"
// #include "BitFunnel/Stream.h"

// TODO: port this to use C++ iterator interface?

namespace BitFunnel
{
    AbstractRowEnumerator::AbstractRowEnumerator(const Term& term,
                                                 IPlanRows& planRows)
        : m_term(&term)
          // m_fact(nullptr)
    {
        // Initialize the RowIds for the match-all and match-none terms for all the shards.
        GetSystemRowIds(planRows);

        // Look up the RowIds for the Term in each Shard and add them to the
        // IPlanRows.
        for (ShardId shard = 0; shard < planRows.GetShardCount(); ++shard)
        {
            RowIdSequence rowIdSequence(term, planRows.GetTermTable(shard));
            ProcessShard(rowIdSequence, planRows, shard);
        }

        FinishInitialization(planRows);
    }


    AbstractRowEnumerator::AbstractRowEnumerator(const FactHandle& fact,
                                                 IPlanRows& planRows)
        : m_term(nullptr)
          // m_fact(&fact)
    {
        // Initialize the RowIds for the match-all and match-none terms for all the shards.
        GetSystemRowIds(planRows);

        // Look up the RowIds for the fact in each Shard and add them to the
        // IPlanRows.
        for (ShardId shard = 0; shard < planRows.GetShardCount(); ++shard)
        {
            RowIdSequence rowIdSequence(fact, planRows.GetTermTable(shard));
            ProcessShard(rowIdSequence, planRows, shard);
        }

        FinishInitialization(planRows);
    }


    void AbstractRowEnumerator::GetSystemRowIds(IPlanRows& planRows)
    {
        // Initialize the RowIds for the match-all and match-none terms for all the shards.
        for (ShardId shard = 0; shard < planRows.GetShardCount(); ++shard)
        {
            RowIdSequence matchAll(ITermTable::GetMatchAllTerm(),
                                   planRows.GetTermTable(shard));
            RowIdSequence matchNone(ITermTable::GetMatchNoneTerm(),
                                    planRows.GetTermTable(shard));

            m_matchAllTermRowIds[shard] = *matchAll.begin();
            m_matchNoneTermRowIds[shard] = *matchNone.begin();
        }
    }


    void AbstractRowEnumerator::ProcessShard(RowIdSequence& rows,
                                             IPlanRows& planRows,
                                             ShardId shard)
    {
        // Storage for the number of RowIds used at each Rank in this Shard.
        unsigned rowsPerRank[c_maxRankValue + 1] = { 0 };

        // Enumerate the RowIds associated with the Term in this Shard.
        auto it = rows.begin();
        while (it != rows.end())
        {
            auto row = *it;
            ++it;
            const Rank rank = row.GetRank();
            RowId physicalRowId = row;

            // Initialize index with the RowId's position in the current Rank.
            unsigned index = rowsPerRank[rank];

            bool slotAvailableForRow = true;

            // If the current Rank has has more rows than the corresponding
            // Rank in the IPlanRows, add another AbstractRow to the plan.
            if (index == m_rows[rank].GetSize())
            {
                if (planRows.IsFull())
                {
                    slotAvailableForRow = false;
                }
                else
                {
                    m_rows[rank].PushBack(planRows.AddRow(rank));
                }
            }

            if (slotAvailableForRow)
            {
                rowsPerRank[rank]++;

                // Update the mapping from (Shard, AbstractRow) to RowId.
                planRows.PhysicalRow(shard, m_rows[rank][index].GetId()) = physicalRowId;
            }
            else
            {
                if (m_term != nullptr)
                {
                    // TODO: log this.
                    // Row limit reached when adding RowIds for a term.
                    // LogB(Logging::Warning,
                    //     "IndexServe",
                    //     "Row count limit reached for term with term hash %I64u "
                    //     "classification %s, GramSize %u, tier %s, as a result row with id"
                    //     "(shard, tier, rank, rowIndex) (%u, %s, %u, %u) is ignored.",
                    //     m_term->GetRawHash(),
                    //     Stream::ClassificationToString(m_term->GetClassification()),
                    //     m_term->GetGramSize(),
                    //     TierToString(m_term->GetTierHint()),
                    //     physicalRowId.GetShard(),
                    //     TierToString(physicalRowId.GetTier()),
                    //     physicalRowId.GetRank(),
                    //     physicalRowId.GetIndex());
                }
                else
                {
                    // TODO: log this.
                    // Row limit reached when adding RowIds for a fact.
                    // LogB(Logging::Warning,
                    //     "IndexServe",
                    //     "Row count limit reached for fact with FactHandle value of %I64u, "
                    //     "as a result row with id"
                    //     "(shard, tier, rank, rowIndex) (%u, %s, %u, %u) is ignored.",
                    //     static_cast<Term::Hash>(*m_fact),
                    //     physicalRowId.GetShard(),
                    //     TierToString(physicalRowId.GetTier()),
                    //     physicalRowId.GetRank(),
                    //     physicalRowId.GetIndex());
                }
            }
        }
    }


    void AbstractRowEnumerator::FinishInitialization(IPlanRows& planRows)
    {
        // Duplicate RowIds in some Shards to ensure that each Rank has the
        // same number of RowIds across all Shards.
        PadWithDuplicateRowIds(planRows);

        // Ensure that the enumerator is ready for its first use.
        Reset();
    }


    void AbstractRowEnumerator::PadWithDuplicateRowIds(IPlanRows& planRows)
    {
        for (ShardId shard = 0; shard < planRows.GetShardCount(); ++shard)
        {
            for (int r = c_maxRankValue; r >= 0; --r)
            {
                Rank rank = static_cast<Rank>(r);
                FixedCapacityVector<AbstractRow, c_maxRowsPerTerm>& rows = m_rows[rank];

                // TODO: What if there are no rows at this rank?
                for (unsigned i = 0; i < rows.GetSize(); ++i)
                {
                    unsigned currentId = rows[i].GetId();
                    if (planRows.PhysicalRow(shard, currentId).IsValid())
                    {
                        continue;
                    }

                    if (i > 0)
                    {
                        unsigned previousId = rows[i - 1].GetId();
                        planRows.PhysicalRow(shard, currentId) =
                            planRows.PhysicalRow(shard, previousId);
                    }
                    else
                    {
                        // All the rows for this rank are invalid rows.
                        // Need to replace it with the match-all row id.
                        planRows.PhysicalRow(shard, currentId) = m_matchAllTermRowIds[shard];
                    }
                }
            }
        }
    }


    bool AbstractRowEnumerator::MoveNext()
    {
        while (m_currentRank >= 0
               && ++m_currentRow >= static_cast<int>(m_rows[m_currentRank].GetSize()))
        {
            --m_currentRank;
            m_currentRow = -1;
        }

        return m_currentRank >= 0;
    }


    AbstractRow AbstractRowEnumerator::Current() const
    {
        LogAssertB(m_currentRow >= 0, "sentinal value in m_currentRow.");

        return m_rows[m_currentRank][static_cast<unsigned>(m_currentRow)];
    }


    void AbstractRowEnumerator::Reset()
    {
        m_currentRank = c_maxRankValue;
        m_currentRow = -1;
    }
}
