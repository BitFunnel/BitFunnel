#pragma once

#include <ostream>

// #include "BitFunnel/BitFunnelTypes.h"         // IdfX10, ShardId, Tier, and Rank parameters.
// #include "BitFunnel/RowId.h"                  // Typpedef RowId is a parameter.
#include "BitFunnel/Term.h"                   // Term::Hash is a parameter.


namespace BitFunnel
{
    class PackedTermInfo;
    class RowId;

    //*************************************************************************
    //
    // ITermTable is an abstract base class or interface for classes providing
    // a mapping from Term to PackedTermInfo. The PackedTermInfo represents
    // a collection of RowId which can be accessed via the TermInfo class.
    //
    // TODO: ITermTable should probably be factored into two separate
    // interfaces - one for build time and one for serve time.
    // TFS 15152.
    //
    //*************************************************************************
    class ITermTable
    {
    public:
        virtual ~ITermTable() {};

        // Writes the TermTable to a stream.
        virtual void Write(std::ostream& stream) const = 0;

        // Adds a single RowId to the term table's RowId buffer.
        virtual void AddRowId(RowId id) = 0;

        // Returns the total number of RowIds currently in the table.
        // Typically used to get record a term's first RowId offset before
        // adding its rows.
        virtual unsigned GetRowIdCount() const = 0;

        // Returns the RowId at the specified offset in the TermTable.
        virtual RowId GetRowId(unsigned rowOffset) const = 0;

        // Returns a RowId for an adhoc term. The resulting RowId takes its
        // shard and tier from the RowId at the specified rowOffset. Its
        // RowIndex is based on a combination of the hash, the variant, and the
        // RowIndex in the RowId at the specified rowOffset.
        //
        // TODO: is the variant parameter really necessary? Isn't it sufficient
        // for the TermTableBuilder to create a set of adhoc RowIds that all
        // have different RowIndex values?
        virtual RowId GetRowIdAdhoc(uint64_t hash, unsigned rowOffset, unsigned variant)  const= 0;

        // Returns a RowId for a fact term. rowOffset specifies an index of the
        // fact in the list of configured facts.
        virtual RowId GetRowIdForFact(unsigned rowOffset) const = 0;

        // Adds a term to the term table. The term's rows must be added first
        // by calling AddRowId().
        virtual void AddTerm(uint64_t hash, unsigned rowIdOffset, unsigned rowIdLength) = 0;

        // Adds an adhoc term to the term table. The design intent is that this
        // method will be called during term table build to supply the set of
        // RowIds that will be used as a pattern for adhoc terms.
        /*
        virtual void AddTermAdhoc(Stream::Classification classification,
                                  unsigned gramSize,
                                  Tier tierHint,
                                  IdfSumX10 idfSum,
                                  unsigned rowIdOffset,
                                  unsigned rowIdLength) = 0;
        */

        // Specifies the total number of rows and the number of shared rows in
        // the row table associated with the specified (tier, rank)
        // values.
        virtual void SetRowTableSize(size_t rank,
                                     unsigned rowCount,
                                     unsigned sharedRowCount) = 0;

        // Returns the total number of rows (private + shared) associated with
        // the row table for (rank). This includes rows allocated for
        // facts, if applicable.
        virtual size_t GetTotalRowCount(size_t rank) const = 0;

        // Returns the number of rows associated with the mutable facts for
        // RowTables with (tier, rank).
        virtual size_t GetMutableFactRowCount(size_t rank) const = 0;

        // Enumeration to represent the type of the term - explicitly stored
        // term, adhoc term or a term which represents a specific fact.
        enum TermKind
        {
            Adhoc,
            Disposed,
            Explicit,
            Fact
        };

        // Returns a PackedTermInfo structure associated with the specified
        // term. The PackedTermInfo structure contains information about the
        // term's rows. PackedTermInfo is used by TermInfo to implement RowId
        // enumeration for regular, adhoc and fact terms.
        virtual PackedTermInfo GetTermInfo(const Term& term, TermKind& termKind) const = 0;

        // Creates a term which is used to mark documents as soft-deleted.
        static Term GetSoftDeletedTerm();

        // Creates a term which is used for padding in the row plan when there
        // are not sufficient or no rows at a particular rank. This term
        // corresponds to a single row at DDR rank 0 which has all bits set to 1.
        static Term GetMatchAllTerm();

        // Creates a term which is used for padding in the row plan, or when
        // handling the requests for rows in tiers which are not supported.
        // This term corresponds to a single row at DDR rank 0 which has all
        // bits set to 0.
        static Term GetMatchNoneTerm();
    };
}
