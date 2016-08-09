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

#include "BitFunnel/Exceptions.h"       // Inline code.
//#include "BitFunnel/Index/IFactSet.h"   // FactHandle is used as a parameter.
#include "BitFunnel/NonCopyable.h"      // TermInfo inherits from NonCopyable.
#include "BitFunnel/RowId.h"            // RowId return value.
#include "BitFunnel/Term.h"             // Embeds Term::Hash.


namespace BitFunnel
{
    class ITermTable2;
    class PackedRowIdSequence;


    //*************************************************************************
    //
    // RowIdSequence
    //
    // Temporary reimplementation of TermInfo. Will replace TermInfo.
    //
    // TermInfo enumerates the RowIds associated with a Term. TermInfo works
    // closely with TermTable and PackedTermInfo to enumerate RowIds.
    //
    //*************************************************************************
    class RowIdSequence : NonCopyable
    {
    public:
        // Constructs a TermInfo for a specified hash, based on information in
        // the supplied ITermTable.
        RowIdSequence(Term const & term, ITermTable2 const & termTable);

        // TODO: Implement this constructor.
        // Constructs a TermInfo for a fact based on information in the
        // supplied ITermTable.
        //RowIdSequence(FactHandle fact, ITermTable const & termTable);

        class const_iterator;
        const_iterator begin() const;
        const_iterator end() const;

        class const_iterator
        {
        public:
            bool operator!=(const_iterator const & other) const
            {
                return (m_current != other.m_current) ||
                       (&m_rowIds != &other.m_rowIds);
            }

            const_iterator& operator++()
            {
                if (m_current == m_rowIds.m_rowIdCount)
                {
                    // We're at the end.
                    RecoverableError error("TermInfo2::operator++: no more RowIds.");
                    throw error;
                }
                ++m_current;
                return *this;
            }

            RowId operator*() const
            {
                if (m_current == m_rowIds.m_rowIdCount)
                {
                    // We're at the end.
                    RecoverableError error("TermInfo2::operator++: no more RowIds.");
                    throw error;
                }
                return m_rowIds.GetRow(m_current);
            }

        private:
            friend class RowIdSequence;

            const_iterator(RowIdSequence const & rowIds, size_t current)
                : m_rowIds(rowIds),
                  m_current(current)
            {
            }

            RowIdSequence const & m_rowIds;
            size_t m_current;
        };

    private:
        // Initializes TermInfo based on the PackedTermInfo returned from the
        // ITermTable.
        void Initialize(PackedRowIdSequence const & packed);

        // Returns the nth row.
        RowId GetRow(size_t n) const;

        // The term's hash is used as a seed for adhoc row enumeration.
        // For non-adhoc terms this value is not applicable and is undefined.
        Term::Hash m_hash;

        // Storing ITermTable as a pointer, rather than a reference to
        // allow for assignment operator.
        const ITermTable2& m_termTable;

        // Slot index for first RowId.
        size_t m_rowIdStart;

        // Number of RowId to be enumerated.
        size_t m_rowIdCount;
    };
}
