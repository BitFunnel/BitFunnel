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

#include "BitFunnel/IInterface.h"           // Base class.
#include "BitFunnel/PackedRowIdSequence.h"  // RowId parameter.
#include "BitFunnel/RowId.h"                // RowId parameter.
#include "BitFunnel/Term.h"                 // Term::Hash parameter.


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
    };
}
