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


#include <stdexcept>

#include "BitFunnel/PackedTermInfo.h"
#include "BitFunnel/Row.h"  // For c_maxRankValue.
#include "BitFunnel/RowId.h"
#include "EmptyTermTable.h"
#include "LoggerInterfaces/Logging.h"
#include "BitFunnel/Term.h"


// TODO: move this somewhere appropriate and make it a real value.
#define c_systemRowCount 3

namespace BitFunnel
{
    EmptyTermTable::EmptyTermTable()
        : m_rowCounts(c_maxRankValue + 1, c_systemRowCount)
    {
        LogAssertB(static_cast<size_t>(m_rowCounts.size()) ==
                   (c_maxRankValue + 1),
                   "");
    }


    EmptyTermTable::EmptyTermTable(std::vector<RowIndex> const & rowCounts)
        : m_rowCounts(rowCounts)
    {
        LogAssertB(static_cast<size_t>(rowCounts.size()) ==
                   (c_maxRankValue + 1),
                   "");
    }


    size_t EmptyTermTable::GetTotalRowCount(size_t rank) const
    {
        return m_rowCounts[rank];
    }


    void EmptyTermTable::Write(std::ostream& /* stream */) const
    {
        throw std::runtime_error("Not implemented");
    }


    void EmptyTermTable::AddRowId(RowId /* id */)
    {
        throw std::runtime_error("Not implemented");
    }


    size_t EmptyTermTable::GetRowIdCount() const
    {
        throw std::runtime_error("Not implemented");
    }


    RowId EmptyTermTable::GetRowId(size_t /* rowOffset */) const
    {
        throw std::runtime_error("Not implemented");
    }


    RowId EmptyTermTable::GetRowIdAdhoc(uint64_t /* hash */,
                                        size_t /* rowOffset */,
                                        size_t /* variant */)  const
    {
        throw std::runtime_error("Not implemented");
    }


    RowId EmptyTermTable::GetRowIdForFact(size_t rowOffset) const
    {
        LogAssertB(m_rowCounts[0] >= c_systemRowCount,
                   "Rank 0 must contain at least c_systemRowCount rows when calling GetRowIdForFact()");

        // EmptyTermTable assumes rows for facts go first.
        LogAssertB(rowOffset < c_systemRowCount,
                   "rowOffset too large.");
        return RowId(0, 0, rowOffset);
    }


    void EmptyTermTable::AddTerm(uint64_t /* hash */,
                                 size_t /* rowIdOffset */,
                                 size_t /* rowIdLength */)
    {
        throw std::runtime_error("Not implemented");
    }


    // void EmptyTermTable::AddTermAdhoc(Stream::Classification /* classification */,
    //                                   unsigned /* gramSize */,
    //                                   Tier /* tierHint */,
    //                                   IdfSumX10 /* idfSum */,
    //                                   unsigned /* rowIdOffset */,
    //                                   unsigned /* rowIdLength */)
    // {
    //     throw std::runtime_error("Not implemented");
    // }


    void EmptyTermTable::SetRowTableSize(size_t /* rank */,
                                         size_t /* rowCount */,
                                         size_t /* sharedRowCount */)
    {
        throw std::runtime_error("Not implemented");
    }


    size_t EmptyTermTable::GetMutableFactRowCount(size_t /* rank */) const
    {
        throw std::runtime_error("Not implemented");
    }


    PackedTermInfo EmptyTermTable::GetTermInfo(const Term& term,
                                               TermKind& termKind) const
    {
        // Soft-deleted row.
        if (static_cast<unsigned>(term.GetRawHash()) < c_systemRowCount)
        {
            LogAssertB(m_rowCounts[0] >= c_systemRowCount,
                       "Rank 0 must contain at least c_systemRowCount rows when calling GetTermInfo()");
            termKind = TermKind::Fact;
            return PackedTermInfo(static_cast<unsigned>(term.GetRawHash()), 1);
        }
        else
        {
            throw std::runtime_error("Not implemented");
        }
    }
}
