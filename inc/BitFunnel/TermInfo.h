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

#include "BitFunnel/IEnumerator.h"        // TermInfo inherits from IEnumerator.
#include "BitFunnel/ITermTable.h"         // Embeds ITermTable::TermKind.
#include "BitFunnel/Index/IFactSet.h"     // FactHandle is used as a parameter.
#include "BitFunnel/NonCopyable.h"        // TermInfo inherits from NonCopyable.
#include "BitFunnel/Term.h"               // Embeds Term::Hash.


namespace BitFunnel
{
    class RowId;
    class TermInfo;

    //*************************************************************************
    //
    // TermInfo
    //
    // TermInfo enumerates the RowIds associated with a Term. TermInfo works
    // closely with TermTable and PackedTermInfo to enumerate RowIds.
    //
    //*************************************************************************
    class TermInfo : public IEnumerator<RowId>, NonCopyable
    {
    public:
        // Constructs a TermInfo for a specified hash, based on information in
        // the supplied ITermTable.
        TermInfo(Term const& term, ITermTable const & termTable);

        // Constructs a TermInfo for a fact based on information in the
        // supplied ITermTable.
        TermInfo(FactHandle fact,  ITermTable const & termTable);

        // Returns true if there are no associated RowIds.
        bool IsEmpty() const;

        // IEnumerator methods.
        bool MoveNext();
        void Reset();
        RowId Current() const;

    private:

        // Initializes TermInfo based on the PackedTermInfo returned from the
        // ITermTable.
        void Initialize(PackedTermInfo const & info);

        // The term's hash is used as a seed for adhoc row enumeration.
        // For non-adhoc terms this value is not applicable and is undefined.
        Term::Hash m_hash;

        // Storing ITermTable as a pointer, rather than a reference to
        // allow for assignment operator.
        const ITermTable& m_termTable;

        // Slot index for first RowId.
        size_t m_rowIdStart;

        // Number of RowId to be enumerated.
        size_t m_rowIdCount;

        // Stores what kind of the term this TermInfo is for.
        ITermTable::TermKind m_termKind;

        // Using int, rather than unsigned to allow for -1 sentinal for
        // position before start of enumerator.
        int64_t m_currentRow;
    };
}
