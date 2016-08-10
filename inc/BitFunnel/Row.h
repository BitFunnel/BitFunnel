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

#include <inttypes.h>  // For uint64_t.

#include "BitFunnel/BitFunnelTypes.h"  // For Rank.
#include "BitFunnel/Index/DocumentHandle.h"  // For DocIndex.

namespace BitFunnel
{
    // Soft-deleted, match-all, and match-none rows.
    static constexpr unsigned c_systemRowCount = 3;

    // The Row class holds a pointer to the data in a bit-vector row.
    // Note that the matching engine may exhibit greater performnce if rows
    // are aligned to quadword (8 byte) boundaries or cache line (64 byte)
    // boundaries. Because the matching engine does quadword loads, it requires
    // that the row length be a multiple of 8 bytes. The row data type is
    // unsigned because we commonly perform bit operations the contents of a
    // row.
    class Row
    {
    public:
        Row(uint64_t const * data);

        uint64_t const * GetData() const;

        // Returns the byte alignment of a row. Rows always start at addresses
        // that are multiples of the byte alignment.
        static unsigned GetAlignment();

        // Returns address rounded up to the next aligned address. Note that
        // address will not be rounded if it is already aligned.
        static uint64_t * Align(void* address);

        // Returns the number of bytes allocated to a row that holds at least
        // documentCount documents.
        static size_t BytesInRow(DocIndex documentCount, Rank rowRank);

        // Returns the actual document capacity of a rank 0 row that has space for
        // at least documentCount documents. The actual capacity will never be less
        // than documentCount, but it might be greater because of padding inserted
        // to ensure alignment of rows at all ranks.
        static DocIndex DocumentsInRank0Row(DocIndex documentCount);

    private:
        // Pointer to the actual row data.
        uint64_t const * m_data;

        // Define the byte alignment for rows.
        // DESIGN NOTE: Row::Align() requires that c_byteAlignment be a power of 2.
        static constexpr unsigned c_log2byteAlignment = 3;
        static constexpr unsigned c_byteAlignment = 1 << c_log2byteAlignment;

        // DESIGN NOTE: Because the matching engine does quadword loads, it
        // is necessary that c_byteAlignment be at least 8 so that
        // DocumentsInRank0Row() correctly pads the row length.
        static_assert(c_byteAlignment >= 8, "c_byteAlignment cannot be less than 8");
    };
}
