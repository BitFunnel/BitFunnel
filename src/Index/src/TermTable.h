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

#include <unordered_map>    // std::unordered_map member.
#include <vector>           // std::vector member.

#include "BitFunnel/ITermTable.h"
#include "BitFunnel/RowId.h"
#include "BitFunnel/Term.h"


namespace BitFunnel
{
    class TermTable : public ITermTable2
    {
    public:
        virtual void OpenTerm() override;

        // Adds a single RowId to the term table's RowId buffer.
        virtual void AddRowId(RowId id) override;

        virtual void CloseTerm(Term::Hash term) override;

        // Returns the total number of rows (private + shared) associated with
        // the row table for (rank). This includes rows allocated for
        // facts, if applicable.
//        virtual size_t GetTotalRowCount(Rank rank) const = 0;

        virtual const_iterator GetRows(Term term) const override;
        virtual const_iterator end() const override;

    private:
        RowIndex m_start;

        // TODO: Is the term table big enough that we would benefit from
        // a more compact class than std::pair<RowIndex, RowIndex>>? Would this
        // even lead to a benefit if we didn't replace std::unordered_map with
        // a better hash table? Should measure actual memory use for this data
        // structure.
        std::unordered_map<Term::Hash, std::pair<RowIndex, RowIndex>> m_termHashToRows;
        std::vector<RowId> m_rowIds;

//        std::vector<RowIndex> m_rowsPerRank;
    };
}
