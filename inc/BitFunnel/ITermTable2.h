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

#include <ostream>

//#include "BitFunnel/BitFunnelTypes.h"       // Rank parameter.
#include "BitFunnel/IInterface.h"           // Base class.
#include "BitFunnel/PackedRowIdSequence.h"  // RowId parameter.
#include "BitFunnel/RowId.h"                // RowId parameter.
#include "BitFunnel/Term.h"                 // Term::Hash parameter.
//#include "ITermTreatment.h"                 // RowConfiguration::Entry::c_maxRowCount.


namespace BitFunnel
{
    class ITermTable2 : public IInterface
    {
    public:
        virtual void OpenTerm() = 0;

        // Adds a single RowId to the term table's RowId buffer.
        virtual void AddRowId(RowId id) = 0;

        virtual void CloseTerm(Term::Hash term) = 0;

        // Returns the total number of rows (private + shared) associated with
        // the row table for (rank). This includes rows allocated for
        // facts, if applicable.
        //virtual size_t GetTotalRowCount(Rank rank) const = 0;

        // Returns a PackedRowIdSequence structure associated with the
        // specified term. The PackedRowIdSequence structure contains
        // information about the term's rows. PackedRowIdSequence is used
        // by RowIdSequence to implement RowId enumeration for regular, adhoc
        // and fact terms.
        virtual PackedRowIdSequence GetRows(const Term& term) const = 0;

        // Returns the RowId at the specified offset in the TermTable.
        virtual RowId GetRowId(size_t rowOffset) const = 0;


        //class PackedRowIdSequence
        //{
        //public:
        //    enum class Type
        //    {
        //        Explicit = 0,
        //        Adhoc = 1,
        //        Fact = 2,
        //        Last = Fact
        //        // WARNING: update c_log2MaxTypeValue, c_maxTypeValue when
        //        // adding new enumeration values to Types.
        //    };
        //    static const size_t c_log2MaxTypeValue = 2;
        //    static const Type c_maxTypeValue = Type::Last;

        //    PackedRowIdSequence(RowIndex start,
        //                        RowIndex end,
        //                        Type type);

        //    RowIndex GetStart() const
        //    {
        //        return m_start;
        //    }

        //    RowIndex GetEnd() const
        //    {
        //        return m_start + m_count;
        //    }

        //    Type GetType() const
        //    {
        //        return static_cast<Type>(m_type);
        //    }

        //private:
        //    const uint32_t m_start : c_log2MaxRowIndexValue;
        //    const uint32_t m_count : RowConfiguration::Entry::c_log2MaxRowCount;
        //    const uint32_t m_type : c_log2MaxTypeValue;
        //};

        //// DESIGN NOTE: PackedRowIdSequence is intended to be a small value
        //// type. Considerations include:
        ////     Term table may potentially store millions of PackedRowIdSequence.
        ////         ==> Small data structure.
        ////     PackedRowIdSequence returned for each term in query
        ////         ==> No memory allocations for copy.
        ////         ==> Value type.
        //static_assert(sizeof(PackedRowIdSequence) == sizeof(uint32_t), "PackedRowIDSequence too large.");
    };
}
