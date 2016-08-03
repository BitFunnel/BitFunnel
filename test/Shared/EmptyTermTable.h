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

#include <vector>

#include "BitFunnel/BitFunnelTypes.h"
#include "BitFunnel/ITermTable.h"
#include "BitFunnel/NonCopyable.h"
#include "BitFunnel/RowId.h"  // For RowIndex.


namespace BitFunnel
{
    class EmptyTermTable : public ITermTable, NonCopyable
    {
    public:
        EmptyTermTable();
        EmptyTermTable(std::vector<RowIndex> const & rowCounts);

        virtual size_t GetTotalRowCount(Rank) const override;

        // These methods are not applicable for the tests and throw if called.
        virtual void Write(std::ostream& stream) const override;
        virtual void AddRowId(RowId id) override;
        virtual uint32_t GetRowIdCount() const override;
        virtual RowId GetRowId(size_t rowOffset) const override;
        virtual RowId GetRowIdAdhoc(Term::Hash hash, size_t rowOffset, size_t variant)  const override;
        virtual RowId GetRowIdForFact(size_t rowOffset) const override;
        virtual void AddTerm(Term::Hash hash, size_t rowIdOffset, size_t rowIdLength) override;
        /*
        virtual void AddTermAdhoc(Stream::Classification classification,
                                  unsigned gramSize,
                                  Tier tierHint,
                                  IdfSumX10 idfSum,
                                  unsigned rowIdOffset,
                                  unsigned rowIdLength) override;
        */
        virtual void SetRowTableSize(Rank rank,
                                     size_t rowCount,
                                     size_t sharedRowCount) override;
        virtual size_t GetMutableFactRowCount(Rank rank) const override;
        virtual PackedTermInfo GetTermInfo(const Term& term, TermKind& termKind) const override;

    private:
        const std::vector<RowIndex> m_rowCounts;

    };
}
