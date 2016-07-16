#include "stdafx.h"

#include "BitFunnel/IFactSet.h"
#include "BitFunnel/PackedTermInfo.h"
#include "MockTermTable.h"
#include "SuiteCpp/UnitTest.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // MockTermTable
    //
    //*************************************************************************
#pragma warning(push)
#pragma warning(disable:4351)
    MockTermTable::MockTermTable(ShardId shard)
        : m_shard(shard),
          m_privateRowCount(),
          m_random0(1234567, 1, 2),
          m_random3(7654321, 0, 4),
          m_random6(1122334, 0, 4)
    {
        // Automatically add system internal rows.
        AddPrivateRowTerm(ITermTable::GetSoftDeletedTerm(), 0);
        AddPrivateRowTerm(ITermTable::GetMatchAllTerm(), 0);
        AddPrivateRowTerm(ITermTable::GetMatchNoneTerm(), 0);
    }


    MockTermTable::MockTermTable(ShardId shard,
        unsigned rank0RowCount,
        unsigned rank3RowCount,
        unsigned rank6RowCount)
        : m_shard(shard),
        m_privateRowCount(),
        m_random0(1234567, rank0RowCount, rank0RowCount),
        m_random3(7654321, rank3RowCount, rank3RowCount),
        m_random6(1122334, rank6RowCount, rank6RowCount)
    {
        // Automatically add system internal rows.
        AddPrivateRowTerm(ITermTable::GetSoftDeletedTerm(), 0);
        AddPrivateRowTerm(ITermTable::GetMatchAllTerm(), 0);
        AddPrivateRowTerm(ITermTable::GetMatchNoneTerm(), 0);
    }
#pragma warning(pop)


    void MockTermTable::Write(std::ostream& /*stream*/) const
    {
        TestFail("MockTermTable::Write() not supported.");
    }


    void MockTermTable::AddRowId(RowId id)
    {
        m_rowIds.push_back(id);
    }


    unsigned MockTermTable::GetRowIdCount() const
    {
        return static_cast<unsigned>(m_rowIds.size());
    }


    RowId MockTermTable::GetRowId(unsigned rowOffset) const
    {
        return m_rowIds[rowOffset];
    }


    RowId MockTermTable::GetRowIdAdhoc(Term::Hash /*hash*/,
                                       unsigned /*rowOffset*/,
                                       unsigned /*variant*/)  const
    {
        TestFail("MockTermTable::GetRowIdAdhoc() not supported.");

        #ifdef _DEBUG
        return RowId();
        #endif /* _DEBUG */
    }


    RowId MockTermTable::GetRowIdForFact(unsigned /* rowOffset */) const
    {
        TestFail("MockTermTable::GetRowIdForFact() not supported.");

        #ifdef _DEBUG
        return RowId();
        #endif /* _DEBUG */
    }


    void MockTermTable::AddTerm(Term::Hash hash,
                                unsigned rowIdOffset,
                                unsigned rowIdLength)
    {
        TestAssert(m_entries.find(hash) == m_entries.end());

        m_entries[hash] = Entry(rowIdOffset, rowIdLength);
    }


    void MockTermTable::AddTermAdhoc(Stream::Classification /*classification*/,
                                     unsigned /*gramSize*/,
                                     Tier /*tierHint*/,
                                     IdfSumX10 /*idfSum*/,
                                     unsigned /*rowIdOffset*/,
                                     unsigned /*rowIdLength*/)
    {
        TestFail("MockTermTable::AddTermAdhoc() not supported.");
    }


    void MockTermTable::SetRowTableSize(Tier /*tier*/,
                                        Rank /*rank*/,
                                        unsigned /*rowCount*/,
                                        unsigned /*sharedRowCount*/)
    {
        TestFail("MockTermTable::SetRowTableSize() not supported.");
    }


    RowIndex MockTermTable::GetTotalRowCount(Tier tier, Rank rank) const
    {
        return m_privateRowCount[tier][rank];
    }


    RowIndex MockTermTable::GetMutableFactRowCount(Tier /*tier*/, Rank /*rank*/) const
    {
        return 0;
    }


    PackedTermInfo MockTermTable::GetTermInfo(const Term& term,
                                              TermKind& termKind) const
    {
        EntryMap::const_iterator it = m_entries.find(term.GetClassifiedHash());

        if (it == m_entries.end())
        {
            //
            // Add any term not already in the term table.
            //

            // First generate some RowIds and add them to the term table.
            unsigned rank0RowCount = m_random0();
            unsigned rank3RowCount = m_random3();
            unsigned rank6RowCount = (rank3RowCount > 0) ? m_random6() : 0;

            unsigned rowIdOffset = GetRowIdCount();

            const Tier tier = term.GetTierHint();
            Rank rank = 0;
            for (unsigned i = 0; i < rank0RowCount; ++i)
            {
                m_rowIds.push_back(RowId(m_shard,
                                   tier,
                                   rank,
                                   m_privateRowCount[tier][rank]++));
            }


            rank = 3;
            for (unsigned i = 0; i < rank3RowCount; ++i)
            {
                m_rowIds.push_back(RowId(m_shard,
                                   tier,
                                   rank,
                                   m_privateRowCount[tier][rank]++));
            }


            rank = 6;
            for (unsigned i = 0; i < rank6RowCount; ++i)
            {
                m_rowIds.push_back(RowId(m_shard,
                                   tier,
                                   rank,
                                   m_privateRowCount[tier][rank]++));
            }

            // Then add the term.
            unsigned rowIdLength = GetRowIdCount() - rowIdOffset;
            m_entries[term.GetClassifiedHash()] = Entry(rowIdOffset, rowIdLength);

            // Update the iterator to point to the newly added entry.
            it = m_entries.find(term.GetClassifiedHash());
        }

        termKind = Explicit;

        return PackedTermInfo(it->second.GetRowIdOffset(),
                              it->second.GetRowIdLength());
    }


    void MockTermTable::AddPrivateRowTerm(Term term, Rank rank)
    {
        const unsigned currentRowOffset = GetRowIdCount();
        m_rowIds.push_back(RowId(m_shard,
                                 DDRTier,
                                 rank,
                                 m_privateRowCount[DDRTier][rank]++));

        AddTerm(term.GetClassifiedHash(), currentRowOffset, 1);
    }


    void MockTermTable::AddRowsForFacts(IFactSet const & facts)
    {
        for (unsigned i = 0; i < facts.GetCount(); ++i)
        {
            // c_systemRowCount rows are reserved as internal.
            const Term term(static_cast<Term::Hash>(i + c_systemRowCount),
                            Stream::MetaWord,
                            0,
                            DDRTier);

            const Rank rankForFact = 0;
            AddPrivateRowTerm(term, rankForFact);
        }
    }


    //*************************************************************************
    //
    // MockTermTable::Entry
    //
    //*************************************************************************
    MockTermTable::Entry::Entry()
        : m_rowIdLength(0),
          m_rowIdOffset(0)
    {
    }


    MockTermTable::Entry::Entry(unsigned rowIdOffset, unsigned rowIdLength)
        : m_rowIdLength(rowIdLength),
          m_rowIdOffset(rowIdOffset)
    {
    }


    unsigned MockTermTable::Entry::GetRowIdOffset() const
    {
        return m_rowIdOffset;
    }



    unsigned MockTermTable::Entry::GetRowIdLength() const
    {
        return m_rowIdLength;
    }

}
