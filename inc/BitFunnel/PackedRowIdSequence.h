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

#include <inttypes.h>                       // For uint*_t.
#include <type_traits>                      // std::is_trivially_copyable in static_assert.

#include "BitFunnel/BitFunnelTypes.h"       // RowIndex parameter.
#include "BitFunnel/Index/ITermTreatment.h" // RowConfiguration::Entry::c_maxRowCount.


namespace BitFunnel
{
    // PackedRowIdSequence defines a sequence of consecutive slots in the
    // TermTable's m_rowIds vector of RowId. Designed to be unpacked and used
    // by the RowIdSequence class which provides a const_iterator. Class
    // TermTable stores PackedRowIdSequence values for Explicit Terms in an
    // std::map. It stores PackedRowIdSequence values for Adhoc Term recipes
    // and an array.
    class PackedRowIdSequence
    {
    public:
        enum class Type
        {
            Explicit = 0,
            Adhoc = 1,
            Fact = 2,
            Last = Fact
            // WARNING: update c_log2MaxTypeValue, c_maxTypeValue when
            // adding new enumeration values to Types.
        };

        static const size_t c_log2MaxTypeValue = 2;
        static const Type c_maxTypeValue = Type::Last;

        // Default constructor creates an empty sequence marked as Type::Adhoc.
        // This constructor is used to initialize the array of Adhoc term
        // recipes in the TermTable.
        PackedRowIdSequence();

        PackedRowIdSequence(RowIndex start,
                            RowIndex end,
                            Type type);

        RowIndex GetStart() const
        {
            return m_start;
        }

        RowIndex GetEnd() const
        {
            return m_start + m_count;
        }

        Type GetType() const
        {
            return static_cast<Type>(m_type);
        }

        // NOTE: Included operator== for write-to-stream/read-from-stream
        // roundtrip test for TermTable. Normally one wants to do black box
        // testing, but it is hard to devise a solid black box test for 
        // TermTable because it is not possible for a test to enumerate all of
        // the terms in the TermTable and all of the adhoc term recipes.
        bool operator==(PackedRowIdSequence const & other) const;

    private:
        // DESIGN NOTE: members would normally be const, but we want this class
        // to be trivially_copyable to allow for binary serialization.
        uint32_t m_start : c_log2MaxRowIndexValue;
        uint32_t m_count : RowConfiguration::Entry::c_log2MaxRowCount;
        uint32_t m_type : c_log2MaxTypeValue;
    };

    // Require PackedRowIdSequence to be trivailly copyable to allow for binary
    // serialization in TermTable.
    static_assert(std::is_trivially_copyable<PackedRowIdSequence >::value,
                  "PackedRowIdSequence must be trivially copyable.");

    // DESIGN NOTE: PackedRowIdSequence is intended to be a small value
    // type. Considerations include:
    //     Term table may potentially store millions of PackedRowIdSequence.
    //         ==> Small data structure.
    //     PackedRowIdSequence returned for each term in query
    //         ==> No memory allocations for copy.
    //         ==> Value type.
    static_assert(sizeof(PackedRowIdSequence) == sizeof(uint32_t), "PackedRowIDSequence too large.");
}
