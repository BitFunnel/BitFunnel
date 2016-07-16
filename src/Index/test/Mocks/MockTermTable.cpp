#include "gtest/gtest.h"

#include "BitFunnel/BitFunnelTypes.h"  // For ShardId.
#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Index/IFactSet.h"
#include "BitFunnel/ITermTable.h"
#include "BitFunnel/PackedTermInfo.h"
#include "BitFunnel/RowId.h"  // For RowIndex.
#include "BitFunnel/Stream.h"
#include "MockTermTable.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // MockTermTable
    //
    //*************************************************************************
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4351)
#endif
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
#ifdef _MSC_VER
#pragma warning(pop)
#endif


    void MockTermTable::Write(std::ostream& /*stream*/) const
    {
        throw NotImplemented();
    }


    void MockTermTable::AddRowId(RowId id)
    {
        m_rowIds.push_back(id);
    }


    unsigned MockTermTable::GetRowIdCount() const
    {
        return static_cast<unsigned>(m_rowIds.size());
    }


    RowId MockTermTable::GetRowId(size_t rowOffset) const
    {
        return m_rowIds[rowOffset];
    }


    RowId MockTermTable::GetRowIdAdhoc(Term::Hash /*hash*/,
                                       size_t /*rowOffset*/,
                                       size_t /*variant*/)  const
    {
        #ifdef _DEBUG
        return RowId();
        #endif /* _DEBUG */

        throw NotImplemented();
    }


    RowId MockTermTable::GetRowIdForFact(size_t /* rowOffset */) const
    {
        #ifdef _DEBUG
        return RowId();
        #endif /* _DEBUG */

        throw NotImplemented();
    }


    void MockTermTable::AddTerm(Term::Hash hash,
                                size_t rowIdOffset,
                                size_t rowIdLength)
    {
        EXPECT_EQ(m_entries.find(hash), m_entries.end());

        m_entries[hash] = Entry(rowIdOffset, rowIdLength);
    }


    void MockTermTable::SetRowTableSize(Rank /*rank*/,
                                        size_t /*rowCount*/,
                                        size_t /*sharedRowCount*/)
    {
        throw NotImplemented();
    }


    RowIndex MockTermTable::GetTotalRowCount(Rank rank) const
    {
        return m_privateRowCount[rank];
    }


    RowIndex MockTermTable::GetMutableFactRowCount(Rank /*rank*/) const
    {
        return 0;
    }


    PackedTermInfo MockTermTable::GetTermInfo(const Term& term,
                                              TermKind& termKind) const
    {
        // TODO: do we need to use GetClassifiedHash here?
        EntryMap::const_iterator it = m_entries.find(term.GetRawHash());

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

            Rank rank = 0;
            for (unsigned i = 0; i < rank0RowCount; ++i)
            {
                m_rowIds.push_back(RowId(m_shard,
                                   rank,
                                   m_privateRowCount[rank]++));
            }


            rank = 3;
            for (unsigned i = 0; i < rank3RowCount; ++i)
            {
                m_rowIds.push_back(RowId(m_shard,
                                   rank,
                                   m_privateRowCount[rank]++));
            }


            rank = 6;
            for (unsigned i = 0; i < rank6RowCount; ++i)
            {
                m_rowIds.push_back(RowId(m_shard,
                                   rank,
                                   m_privateRowCount[rank]++));
            }

            // Then add the term.
            unsigned rowIdLength = GetRowIdCount() - rowIdOffset;
            m_entries[term.GetRawHash()] = Entry(rowIdOffset, rowIdLength);

            // Update the iterator to point to the newly added entry.
            it = m_entries.find(term.GetRawHash());
        }

        termKind = Explicit;

        return PackedTermInfo(it->second.GetRowIdOffset(),
                              it->second.GetRowIdLength());
    }


    void MockTermTable::AddPrivateRowTerm(Term term, Rank rank)
    {
        const unsigned currentRowOffset = GetRowIdCount();
        m_rowIds.push_back(RowId(m_shard,
                                 rank,
                                 m_privateRowCount[rank]++));

        AddTerm(term.GetRawHash(), currentRowOffset, 1);
    }


    void MockTermTable::AddRowsForFacts(IFactSet const & facts)
    {
        for (unsigned i = 0; i < facts.GetCount(); ++i)
        {
            // c_systemRowCount rows are reserved as internal.
            const Term term(static_cast<Term::Hash>(i + c_systemRowCount),
                            StreamId::MetaWord,
                            0);

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
        : m_rowIdOffset(0),
          m_rowIdLength(0)
    {
    }


    MockTermTable::Entry::Entry(unsigned rowIdOffset, unsigned rowIdLength)
        : m_rowIdOffset(rowIdOffset),
          m_rowIdLength(rowIdLength)
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
