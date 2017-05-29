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

#include <inttypes.h>                   // For uint32_t.
#include <type_traits>                  // std::is_trivially_copyable in static_assert.

#include "BitFunnel/BitFunnelTypes.h"   // Rank, RowIndex, ShardId parameters.


namespace BitFunnel
{
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
        // Constructs a RowId with shard, rank, and index initialized to 0.
        // Default constructor used to initialized std::vector in
        // StreamUtilities::ReadVector().
        RowId();

        // Constructor for primary use case.
        // TODO: Replace size_t with ShardID, Rank, RowIndex.
        // TODO: Why do we need shard?
        RowId(Rank rank, RowIndex index, bool isAdhoc = false);

        // Constructs a new RowId by adding index to the RowIndex of an
        // existing RowId. Used by the TermTable::Seal().
        RowId(const RowId& other, RowIndex index);

        // Return's the row's Rank.
        Rank GetRank() const;

        // Returns the row's Index.
        RowIndex GetIndex() const;

        bool IsAdhoc() const;

        // Equality operators used in unit tests.
        bool operator==(const RowId& other) const;
        bool operator!=(const RowId& other) const;
        bool operator<(const RowId& other) const;

        // Returns true if the row is considered valid.
        bool IsValid() const;

    private:
        static_assert(c_log2MaxRankValue +      // m_rank
                      c_log2MaxRowIndexValue +  // m_index
                      1 +                       // m_isAdhoc
                      1                         // m_isValid
                      <= 32ull,
                      "Expect m_rank and m_index to use no more than 32 bits.");

        // DESIGN NOTE: members would normally be const, but we want this class
        // to be trivially_copyable to allow for binary serialization.

        // Rank.
        uint32_t m_rank: c_log2MaxRankValue;

        // Index is the row number within a row table.
        // We are limited to c_log2MaxRowIndexValue rows, which at this time
        // is 2^25 = 33M rows. At 10% bit density this means that BitFunnel
        // isi limited to 3.3M postings per document.
        uint32_t m_index: c_log2MaxRowIndexValue;

        uint32_t m_isAdhoc : 1;

        uint32_t m_isValid : 1;

        // We fill in unused bits to prevent valgrind from complaining when we
        // serialize this data structure to disk.
        uint32_t m_unused :
            32 - c_log2MaxRankValue - c_log2MaxRowIndexValue - 1 - 1;
    };

    static_assert(sizeof(RowId) == 4,
                  "Expect sizeof(RowId) to be 4 bytes.");

    // Require RowId to be trivailly copyable to allow for binary serialization
    // in TermTable.
    static_assert(std::is_trivially_copyable<RowId>::value,
                  "RowId must be trivially copyable.");
}
