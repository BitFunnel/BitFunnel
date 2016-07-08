// #include <istream>
// #include <ostream>
// #include <sstream>

// #include "Array.h"
// #include "BitFunnel/BitFunnelTypes.h"
// #include "BitFunnel/Factories.h"
// #include "BitFunnel/FileHeader.h"
// #include "BitFunnel/IFactSet.h"
// #include "BitFunnel/ITermDisposeDefinition.h"
// #include "BitFunnel/PackedTermInfo.h"
// #include "BitFunnel/RowId.h"
// #include "BitFunnel/StreamUtilities.h"
// #include "BitFunnel/Version.h"
// #include "MurmurHash2.h"
// #include "PackedArray.h"
// #include "SimpleHashTable.h"
// #include "TermTable.h"

#include "BitFunnel/ITermTable.h"
#include "BitFunnel/Row.h"
#include "BitFunnel/Term.h"
#include "BitFunnel/Stream.h"
#include "LoggerInterfaces/Logging.h"


namespace BitFunnel
{
    // namespace Factories
    // {
    //     ITermTable* CreateTermTable(const ITermDisposeDefinition* termDisposeDefinition,
    //                                 unsigned termCount,
    //                                 unsigned rowIdCapacity)
    //     {
    //         return new TermTable(termDisposeDefinition, termCount, rowIdCapacity);
    //     }


    //     const ITermTable* CreateTermTable(std::istream& input,
    //                                       IFactSet const & facts)
    //     {
    //         return new TermTable(input, facts);
    //     }
    // }

    // //*****************************************************************************
    // //
    // // TermTable
    // //
    // //*****************************************************************************
    // const Version TermTable::c_version(1, 0, 0);


    // TermTable::TermTable(const ITermDisposeDefinition* termDisposeDefinition,
    //                      unsigned termCount,
    //                      unsigned rowIdCapacity)
    //     : m_termDisposeDefinition(termDisposeDefinition),
    //       m_hashToPackedTermInfo(new HashTable(static_cast<unsigned>(termCount * 1.4),
    //                                            false)),
    //       m_rowIdBuffer(new PackedArray(rowIdCapacity,
    //                                     RowId::GetPackedRepresentationBitCount(),
    //                                     false)),
    //       m_rowIdCount(0),
    //       m_privateRowCounts(new UnsignedRankTierArray()),
    //       m_sharedRowCounts(new UnsignedRankTierArray()),
    //       m_factsCount(0)   // Facts are not used in the TermTable building scenario.
    // {
    //     for (unsigned i = 0; i <= c_maxIdfSumX10Value; i++)
    //     {
    //         const IdfSumX10 idfSum = static_cast<IdfSumX10>(i);
    //         m_adhocTermInfos[idfSum].reset(new AdhocTermInfoArray());
    //     }
    // }


    // TermTable::TermTable(std::istream& input,
    //                      IFactSet const & facts)
    //     : m_factsCount(facts.GetCount() + c_systemRowCount)    // See comment below.
    // {
    //     // System internal rows are handled in the same way as Facts, even though
    //     // they are not part of the IFactSet and are not visible to the user.
    //     // Fact count is increased by the number of system rows accordingly.

    //     FileHeader fileHeader(input);
    //     LogAssertB(fileHeader.GetVersion().IsCompatibleWith(c_version));

    //     std::string termDisposeStr;
    //     StreamUtilities::ReadString(input, termDisposeStr);

    //     std::stringstream termDisposeStream(termDisposeStr);

    //     m_termDisposeDefinition = Factories::CreateTermDisposeDefinition(termDisposeStream);
    //     m_termDisposeDefinitionPtr.reset(m_termDisposeDefinition);

    //     m_hashToPackedTermInfo.reset(new HashTable(input));
    //     for (unsigned i = 0; i <= c_maxIdfSumX10Value; i++)
    //     {
    //         const IdfSumX10 idfSum = static_cast<IdfSumX10>(i);
    //         m_adhocTermInfos[idfSum].reset(new AdhocTermInfoArray(input));
    //     }

    //     m_rowIdBuffer.reset(new PackedArray(input));
    //     m_rowIdCount = static_cast<unsigned>(m_rowIdBuffer->GetCapacity());

    //     m_privateRowCounts.reset(new UnsignedRankTierArray(input));
    //     m_sharedRowCounts.reset(new UnsignedRankTierArray(input));
    // }


    // void TermTable::Write(std::ostream& output) const
    // {
    //     FileHeader fileHeader(c_version, "TermTable");
    //     fileHeader.Write(output);

    //     std::stringstream termDisposeStream;
    //     m_termDisposeDefinition->Write(termDisposeStream);

    //     StreamUtilities::WriteString(output, termDisposeStream.str());

    //     m_hashToPackedTermInfo->Write(output);
    //     for (unsigned i = 0; i <= c_maxIdfSumX10Value; i++)
    //     {
    //         const IdfSumX10 idfSum = static_cast<IdfSumX10>(i);
    //         m_adhocTermInfos[idfSum]->Write(output);
    //     }

    //     m_rowIdBuffer->Write(output);

    //     m_privateRowCounts->Write(output);
    //     m_sharedRowCounts->Write(output);
    // }


    // void TermTable::AddRowId(RowId rowId)
    // {
    //     m_rowIdBuffer->Set(m_rowIdCount++, rowId.GetPackedRepresentation());
    // }


    // unsigned TermTable::GetRowIdCount() const
    // {
    //     return m_rowIdCount;
    // }


    // RowId TermTable::GetRowId(unsigned rowOffset) const
    // {
    //     return RowId(m_rowIdBuffer->Get(rowOffset));
    // }


    // RowId TermTable::GetRowIdAdhoc(Term::Hash hash,
    //                                unsigned rowOffset,
    //                                unsigned variant) const
    // {
    //     const RowId rowId(GetRowId(rowOffset));

    //     const ShardId shard = rowId.GetShard();
    //     const Tier tier = rowId.GetTier();
    //     const Rank rank = rowId.GetRank();
    //     const RowIndex index = rowId.GetIndex();

    //     const unsigned sharedRowCount = m_sharedRowCounts->At(tier, rank);

    //     // Derive adhoc row index from a combination of the term hash and the
    //     // supplied row's index and variant. Want to ensure that all rows
    //     // generated for the same (ShardId, Tier, Rank) are different.
    //     hash = _rotl64(hash, index % 64);
    //     hash += variant;

    //     return RowId(shard, tier, rank, (hash % sharedRowCount));
    // }


    // RowId TermTable::GetRowIdForFact(unsigned rowOffset) const
    // {
    //     // System internal rows are already included in m_factsCount, hence
    //     // deliberately using "less than".
    //     LogAssertB(rowOffset < m_factsCount);

    //     const RowIndex rowCountDdr0 = m_privateRowCounts->At(DDRTier, 0)
    //                                   + m_sharedRowCounts->At(DDRTier, 0);

    //     // AnyRow is used to get a shard.
    //     // TODO: investigate if we should split RowId to shard +
    //     // shard-independent structure and keep m_shard in the TermTable.
    //     // TFS 15153.
    //     const RowId anyRow = GetRowId(0);

    //     // Soft-deleted document row is the first one after all regular
    //     // rows. The caller specifies rowOffset = 0 in this case. The rationale
    //     // for this value is that a soft-deleted rowId must be consistent
    //     // between query planner and query runner regardless of the number of
    //     // user facts defined. This is to guarantee consistency of this row
    //     // in case of canary deployment of the code that changes the list of
    //     // facts.
    //     return RowId(anyRow.GetShard(), DDRTier, 0, rowCountDdr0 + rowOffset);
    // }


    // void TermTable::AddTerm(Term::Hash hash, unsigned rowIdOffset, unsigned rowIdLength)
    // {
    //     (*m_hashToPackedTermInfo)[hash] = PackedTermInfo(rowIdOffset, rowIdLength);
    // }


    // void TermTable::AddTermAdhoc(Stream::Classification classification,
    //                              unsigned gramSize,
    //                              Tier tierHint,
    //                              IdfSumX10 idfSum,
    //                              unsigned rowIdOffset,
    //                              unsigned rowIdLength)
    // {
    //     LogAssertB(idfSum <= c_maxIdfSumX10Value);

    //     m_adhocTermInfos[idfSum]->At(static_cast<unsigned>(classification),
    //                                  gramSize,
    //                                  tierHint) = PackedTermInfo(rowIdOffset, rowIdLength);
    // }


    // void TermTable::SetRowTableSize(Tier tier,
    //                                 Rank rank,
    //                                 unsigned privateRowCount,
    //                                 unsigned sharedRowCount)
    // {
    //     LogAssertB(tier < TierCount);
    //     LogAssertB(rank <= c_maxRankValue);

    //     m_privateRowCounts->At(tier, rank) = privateRowCount;
    //     m_sharedRowCounts->At(tier, rank) = sharedRowCount;
    // }


    // RowIndex TermTable::GetTotalRowCount(Tier tier, Rank rank) const
    // {
    //     LogAssertB(tier < TierCount);
    //     LogAssertB(rank <= c_maxRankValue);

    //     // Total number of rows is private + shared + mutable facts rows.
    //     return m_privateRowCounts->At(tier, rank)
    //            + m_sharedRowCounts->At(tier, rank)
    //            + GetMutableFactRowCount(tier, rank);
    // }


    // RowIndex TermTable::GetMutableFactRowCount(Tier tier, Rank rank) const
    // {
    //     // All facts are treated as mutable now. Future implementation may
    //     // separate mutable vs non-mutable; Also facts are only DDR rank 0
    //     // now. If IFactSet is expanded to other tiers or ranks, it can expose
    //     // a method to GetCount(Tier, Rank).
    //     return (tier == DDRTier && rank == 0) ? m_factsCount : 0;
    // }


    // PackedTermInfo TermTable::GetTermInfo(const Term& term, TermKind& termKind) const
    // {
    //     // TODO: check tierHint/classification too? This can reduce probability
    //     // of collisions too.

    //     // m_factsCount = count from IFactSet + c_systemRowCount for system
    //     // internal rows, hence deliberately using "less than" here.
    //     if (term.GetRawHash() < m_factsCount)
    //     {
    //         // Facts are private rank 0 rows with the special row offsets.
    //         termKind = TermKind::Fact;
    //         const unsigned factIndex = static_cast<unsigned>(term.GetRawHash());
    //         return PackedTermInfo(factIndex, 1);
    //     }
    //     else
    //     {
    //         bool found = false;
    //         PackedTermInfo& info = m_hashToPackedTermInfo->Find(term.GetClassifiedHash(),
    //                                                             found);

    //         if (found)
    //         {
    //             termKind = TermKind::Explicit;
    //             return info;
    //         }
    //         else if (m_termDisposeDefinition->IsDispose(term.GetGramSize(),
    //                                                     term.GetIdfSum(),
    //                                                     term.GetTierHint()))
    //         {
    //             termKind = TermKind::Disposed;
    //             return m_disposedTermInfo;
    //         }
    //         else
    //         {
    //             termKind = TermKind::Adhoc;
    //             const IdfSumX10 idfSum = term.GetIdfSum();
    //             return m_adhocTermInfos[idfSum]->At(static_cast<unsigned>(term.GetClassification()),
    //                                                 term.GetGramSize(),
    //                                                 term.GetTierHint());
    //         }

    //     }
    // }


    // Helper function to create a system internal term. System internal terms
    // are special terms whose Raw hash values by convention are sequential
    // numbers starting at 0. The number of such terms is specified in
    // c_systemRowCount.
    Term CreateSystemInternalTerm(Term::Hash rawHash)
    {
        LogAssertB(rawHash < c_systemRowCount, "Hash out of range.");

        const Term term(rawHash,
                        StreamId::MetaWord,
                        static_cast<Term::IdfX10>(0));
        return term;
    }


    Term ITermTable::GetSoftDeletedTerm()
    {
        static Term softDeletedTerm(CreateSystemInternalTerm(0));
        return softDeletedTerm;
    }


    Term ITermTable::GetMatchAllTerm()
    {
        static Term matchAllTerm(CreateSystemInternalTerm(1));
        return matchAllTerm;
    }


    Term ITermTable::GetMatchNoneTerm()
    {
        static Term matchNoneTerm(CreateSystemInternalTerm(2));
        return matchNoneTerm;
    }
}
