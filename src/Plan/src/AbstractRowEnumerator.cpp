#include "stdafx.h"

#include "AbstractRowEnumerator.h"
#include "BitFunnel/IPlanRows.h"
#include "BitFunnel/Stream.h"
#include "BitFunnel/TermInfo.h"
#include "BitFunnel/Tier.h"
#include "LoggerInterfaces/Logging.h"


namespace BitFunnel
{
    // Return true if a tier is serving index.
    static bool IsServingTier(Tier tier)
    {
        return tier == DDRTier;
    }


    AbstractRowEnumerator::AbstractRowEnumerator(const Term& term,
                                                 IPlanRows& planRows)
        : m_term(&term),
          m_fact(nullptr)
    {
        // Initialize the RowIds for the match-all and match-none terms for all the shards.
        GetSystemRowIds(planRows);

        // Look up the RowIds for the Term in each Shard and add them to the
        // IPlanRows.
        for (ShardId shard = 0; shard < planRows.GetShardCount(); ++shard)
        {
            TermInfo termInfo(term, planRows.GetTermTable(shard));
            ProcessShard(termInfo, planRows, shard);
        }

        FinishInitialization(planRows);
    }


    AbstractRowEnumerator::AbstractRowEnumerator(const FactHandle& fact,
                                                 IPlanRows& planRows)
        : m_term(nullptr),
          m_fact(&fact)
    {
        // Initialize the RowIds for the match-all and match-none terms for all the shards.
        GetSystemRowIds(planRows);

        // Look up the RowIds for the fact in each Shard and add them to the
        // IPlanRows.
        for (ShardId shard = 0; shard < planRows.GetShardCount(); ++shard)
        {
            TermInfo termInfo(fact, planRows.GetTermTable(shard));
            ProcessShard(termInfo, planRows, shard);
        }

        FinishInitialization(planRows);
    }


    void AbstractRowEnumerator::GetSystemRowIds(IPlanRows& planRows)
    {
        // Initialize the RowIds for the match-all and match-none terms for all the shards.
        for (ShardId shard = 0; shard < planRows.GetShardCount(); ++shard)
        {
            TermInfo matchAllTermInfo(ITermTable::GetMatchAllTerm(), planRows.GetTermTable(shard));
            TermInfo matchNoneTermInfo(ITermTable::GetMatchNoneTerm(), planRows.GetTermTable(shard));

            LogAssertB(matchAllTermInfo.MoveNext());
            m_matchAllTermRowIds[shard] = matchAllTermInfo.Current();
            LogAssertB(!matchAllTermInfo.MoveNext())

            LogAssertB(matchNoneTermInfo.MoveNext());
            m_matchNoneTermRowIds[shard] = matchNoneTermInfo.Current();
            LogAssertB(!matchNoneTermInfo.MoveNext())
        }
    }


    void AbstractRowEnumerator::ProcessShard(TermInfo& termInfo,
                                             IPlanRows& planRows,
                                             ShardId shard)
    {
        // Storage for the number of RowIds used at each Rank in this Shard.
        unsigned rowsPerRank[c_maxRankValue + 1] = { 0 };

        // Enumerate the RowIds associated with the Term in this Shard.
        while (termInfo.MoveNext())
        {
            const RowId rowId = termInfo.Current();
            const Rank rank = rowId.GetRank();

            RowId physicalRowId = rowId;

            if (!IsServingTier(rowId.GetTier()))
            {
                physicalRowId = m_matchNoneTermRowIds[shard];
                LogB(Logging::Info,
                    "BitFunnelQueryPlanning",
                    "Row (shard, tier, rank, rowIndex) (%u, %s, %u, %u) for term with "
                    "term hash %I64u, classification %s, GramSize %u, tier %s is in "
                    "a non-serving tier, this row is replaced with match all term row.",
                    rowId.GetShard(),
                    TierToString(rowId.GetTier()),
                    rowId.GetRank(),
                    rowId.GetIndex(),
                    m_term->GetRawHash(),
                    Stream::ClassificationToString(m_term->GetClassification()),
                    m_term->GetGramSize(),
                    TierToString(m_term->GetTierHint()));
            }

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
                    // Row limit reached when adding RowIds for a term.
                    LogB(Logging::Warning,
                        "IndexServe",
                        "Row count limit reached for term with term hash %I64u "
                        "classification %s, GramSize %u, tier %s, as a result row with id"
                        "(shard, tier, rank, rowIndex) (%u, %s, %u, %u) is ignored.",
                        m_term->GetRawHash(),
                        Stream::ClassificationToString(m_term->GetClassification()),
                        m_term->GetGramSize(),
                        TierToString(m_term->GetTierHint()),
                        physicalRowId.GetShard(),
                        TierToString(physicalRowId.GetTier()),
                        physicalRowId.GetRank(),
                        physicalRowId.GetIndex());
                }
                else
                {
                    // Row limit reached when adding RowIds for a fact.
                    LogB(Logging::Warning,
                        "IndexServe",
                        "Row count limit reached for fact with FactHandle value of %I64u, "
                        "as a result row with id"
                        "(shard, tier, rank, rowIndex) (%u, %s, %u, %u) is ignored.",
                        static_cast<Term::Hash>(*m_fact),
                        physicalRowId.GetShard(),
                        TierToString(physicalRowId.GetTier()),
                        physicalRowId.GetRank(),
                        physicalRowId.GetIndex());
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
        LogAssertB(m_currentRow >= 0);

        return m_rows[m_currentRank][m_currentRow];
    }


    void AbstractRowEnumerator::Reset()
    {
        m_currentRank = c_maxRankValue;
        m_currentRow = -1;
    }
}
