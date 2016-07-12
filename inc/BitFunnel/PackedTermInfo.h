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

#include <inttypes.h>  // For uint8_t.
#include <stddef.h>  // For size_t.

// #include "BitFunnel/BitFunnelTypes.h"     // PackedTermInfo embeds RowIndex.
// #include "BitFunnel/RowId.h"              // For RowIndex.

namespace BitFunnel
{
    class Term;

    //*************************************************************************
    //
    // PackedTermInfo
    //
    // PackedTermInfo stores the RowIds associated with a Term in the context
    // of some TermTable. The RowIds are represented as a contiguous sequence
    // of slots in the TermTable's RowId buffer.
    //
    // The TermInfo class decodes the PackedTermInfo in the context of a
    // particular TermTable.
    //
    // DESIGN NOTE: PackedTermInfo is a POD type (plain old data) and is
    // intended to be passed and returned as a value type.
    //
    //*************************************************************************
    // TODO: packing.
    //#pragma pack(push, 1)
      // maximally compact object, optimize size over performance.
    //    __declspec (align(1))
    class PackedTermInfo
    {
    public:
        // Constructs a PackedTermInfo initialized with provided values.
        PackedTermInfo(unsigned rowIdStart, unsigned rowIdCount);

        // Constructs an empty PackTermInfo() with m_rowIdCount == 0.
        // Mainly used for creating SimpleHashTable<PackedTermInfo> in TermTable.
        PackedTermInfo();

        // Returns true if m_rowIdCount == 0.
        bool IsEmpty() const;

        // Returns the offset of the first row in the TermTable's RowId table.
        size_t GetRowIdStart() const;

        // Returns the number of consecutive rows in the TermTable's RowId table.
        unsigned GetRowIdCount() const;

        // Returns true the objects are equal.
        bool operator==(const PackedTermInfo& rhs) const;

    private:
        // Index into the TermTable's RowId table.
        size_t m_rowIdStart;

        // Number of consecutive RowIds
        uint8_t m_rowIdCount;
    };
    // TODO: packing.
    // #pragma pack(pop)
}
