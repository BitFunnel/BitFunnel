#pragma once

#include <string>

// TODO: fix obsolete comments (e.g., comment that RowId is <= 32 bits).

namespace BitFunnel
{
    typedef size_t RowIndex;

    //*************************************************************************
    //
    // RowId is a unique identifier for a row in an index.
    //
    // DESIGN NOTE: RowId is intended to be used as a value type. It currently
    // uses only 32 bits. RowId is designed to be compact for storate in
    // memory tables.
    //
    //*************************************************************************
    class RowId
    {
    public:
        // Default constructor creates invalid rows
        // (i.e. IsValid() will return false).
        RowId();

        // Constructor for primary use case.
        RowId(size_t shard, size_t rank, size_t index);

        // RowId is used as a value type and is often copied. Cannot generate
        // default copy constructor because of bit fields.
        // Primary copy scenario is in PlanRow::operator[]. Other usage is
        // TermAllocator::ExportTermTable.
        RowId(const RowId& other);

        // Constructs a RowId from a 40-bit packed representation.
        // DESIGN NOTE: The packed representation provides more compact storage
        // for the RowId fields than can be accomplished in the RowId class.
        // The layout of the bit fields in RowId is compiler dependent and in
        // the case of the compiler, the fields occupy 64 bits of space, even
        // though only 40 bits are defined.
        RowId(uint64_t packedRepresentation);

        // Returns the 40-bit packed representation of the RowId.
        uint64_t GetPackedRepresentation() const;

        // Return's the row's Rank.
        size_t GetRank() const;

        // Returns the row's Index.
        size_t GetIndex() const;

        // Returns the row's ShardId.
        size_t GetShard() const;

        // Equality operators used in unit tests.
        bool operator==(const RowId& other) const;
        bool operator!=(const RowId& other) const;
        bool operator<(const RowId& other) const;

        // Returns true if the row is considered valid.
        bool IsValid() const;

        // Returns the number of significant bits in the packed representation
        // of a RowId. Currently this method returns the value 40.
        static unsigned GetPackedRepresentationBitCount();

    protected:
        // The following constants define the 40 bits of RowId assigned to
        // Tier, Rank, ShardId, Index. Constants are protected to allow access
        // from the unit test.
        static const unsigned c_bitsOfShard = 4;
        static const unsigned c_bitsOfRank = 3;
        static const unsigned c_bitsOfIndex = 31;

    public:
        static const size_t c_maxRowIndexValue = (1ul << c_bitsOfIndex) - 1;

    private:
        // ShardId number.
        unsigned m_shard: c_bitsOfShard;

        // Rank.
        unsigned m_rank: c_bitsOfRank;

        // Index is the row number within a row table.
        // We are limited to 8M rows because of the 32 bit size of RowId. At
        // 10% bit density that means that bit funnel is limited to 800K
        // postings per document per tier per rank or approximately 100K terms
        // in a document
        unsigned m_index: c_bitsOfIndex;
    };
}
