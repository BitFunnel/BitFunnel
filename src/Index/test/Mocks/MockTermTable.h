#pragma once

#include <map>
#include <vector>

#include "BitFunnel/BitFunnelTypes.h"  // For Rank, ShardId
#include "BitFunnel/ITermTable.h"
#include "BitFunnel/Row.h"  // For c_maxRankValue.
#include "BitFunnel/RowId.h"  // For RowIndex.
#include "Random.h"


namespace BitFunnel
{
    class IFactSet;

    class MockTermTable : public ITermTable
    {
    public:
        MockTermTable(ShardId shard);

        // Create a term table which always allocate
        // a fixed number of rows for various ranks.
        MockTermTable(ShardId shard,
            unsigned rank0RowCount,
            unsigned rank3RowCount,
            unsigned rank6RowCount);

        // Add a term and reserves a private row for it.
        void AddPrivateRowTerm(Term term, Rank rank);

        // Allocates rows which are used for Facts.
        void AddRowsForFacts(IFactSet const & facts);

        //
        // ITermTable methods.
        //

        // Not supported. Throws an exception.
        virtual void Write(std::ostream& stream) const override;

        // Adds a single RowId to the term table's RowId buffer.
        virtual void AddRowId(RowId id) override;

        // Returns the total number of RowIds currently in the table.
        // Typically used to get record a term's first RowId offset before
        // adding its rows.
        virtual unsigned GetRowIdCount() const override;

        // Returns the RowId at the specified offset in the TermTable.
        virtual RowId GetRowId(size_t rowOffset) const override;

        // Not supported. Throws an exception.
        // This method is not necessary because the MockTermTable does not
        // return adhoc terms. All calls to GetTermInfo() will result in the
        // term being added to the TermTable.
        virtual RowId GetRowIdAdhoc(Term::Hash hash,
                                    size_t rowOffset,
                                    size_t variant)  const override;

        // Not supported. Throws an exception.
        virtual RowId GetRowIdForFact(size_t rowOffset) const override;

        // Adds a term to the term table. The term's rows must be added first
        // by calling AddRowId().
        virtual void AddTerm(Term::Hash hash,
                             size_t rowIdOffset,
                             size_t rowIdLength) override;

        /*
        // Not supported. Throws an exception.
        virtual void AddTermAdhoc(Stream::Classification classification,
                                  unsigned gramSize,
                                  IdfSumX10 idfSum,
                                  unsigned rowIdOffset,
                                  unsigned rowIdLength) override;
        */

        // Not supported. Throws an exception.
        // RowTable sizes are updated each time an explicit term is added.
        virtual void SetRowTableSize(Rank rank,
                                     size_t rowCount,
                                     size_t sharedRowCount) override;

        // Returns the total number of rows (private + shared) associated with
        // the row table for (tier, rank).
        virtual RowIndex GetTotalRowCount(Rank rank) const override;

        // Returns the number of rows associated with the mutable facts for
        // RowTables with (tier, rank).
        virtual RowIndex GetMutableFactRowCount(Rank rank) const override;

        // Returns a PackedTermInfo structure associated with the specified
        // term. The PackedTermInfo structure contains information about the
        // term's rows. PackedTermInfo is used by TermInfo to implement RowId
        // enumeration for regular and adhoc terms.
        virtual PackedTermInfo GetTermInfo(const Term& term,
                                           TermKind& termKind) const override;

    private:

        class Entry
        {
        public:
            Entry();
            Entry(unsigned rowIdOffset, unsigned rowIdLength);

            unsigned GetRowIdOffset() const;
            unsigned GetRowIdLength() const;

        private:
            unsigned m_rowIdOffset;
            unsigned m_rowIdLength;
        };

        ShardId m_shard;

        mutable std::vector<RowId> m_rowIds;

        typedef std::map<Term::Hash, Entry> EntryMap;
        mutable EntryMap m_entries;

        typedef unsigned RankArray[c_maxRankValue + 1];
        mutable RankArray m_privateRowCount;

        mutable RandomInt<unsigned> m_random0;
        mutable RandomInt<unsigned> m_random3;
        mutable RandomInt<unsigned> m_random6;
    };
}
