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


#pragma once

#include <string>

#include "BitFunnel/BitFunnelTypes.h"  // For Rank, ShardId.


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
        // TODO: Replace size_t with ShardID, Rank, RowIndex.
        // TODO: Why do we need shard?
        RowId(size_t shard, size_t rank, size_t index);

        // RowId is used as a value type and is often copied. Cannot generate
        // default copy constructor because of bit fields.
        // Primary copy scenario is in PlanRow::operator[]. Other usage is
        // TermAllocator::ExportTermTable.
        RowId(const RowId& other);

        // Constructs a RowId from a 32-bit packed representation.
        // DESIGN NOTE: The packed representation provides more compact storage
        // for the RowId fields than can be accomplished in the RowId class.
        // The layout of the bit fields in RowId is compiler dependent and in
        // the case of the compiler, the fields occupy 64 bits of space, even
        // though only 32 bits are defined.
        RowId(uint32_t packedRepresentation);

        // Returns the 32-bit packed representation of the RowId.
        uint32_t GetPackedRepresentation() const;

        // Return's the row's Rank.
        Rank GetRank() const;

        // Returns the row's Index.
        RowIndex GetIndex() const;

        // Returns the row's ShardId.
        ShardId GetShard() const;

        // Equality operators used in unit tests.
        bool operator==(const RowId& other) const;
        bool operator!=(const RowId& other) const;
        bool operator<(const RowId& other) const;

        // Returns true if the row is considered valid.
        bool IsValid() const;

        // Returns the number of significant bits in the packed representation
        // of a RowId. Currently this method returns the value 32.
        static unsigned GetPackedRepresentationBitCount();

    protected:
        // The following constants define the 32-bits of RowId assigned to
        // Tier, Rank, ShardId, Index. Constants are protected to allow access
        // from the unit test.
        static const unsigned c_bitsOfShard = 4;
        static const unsigned c_bitsOfRank = 3;
        static const unsigned c_bitsOfIndex = 32 - c_bitsOfShard - c_bitsOfRank;

    public:
        static const size_t c_maxRowIndexValue = (1ul << c_bitsOfIndex) - 1;
        static const size_t c_maxRankValue = (1ul << c_bitsOfRank) - 1;

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
