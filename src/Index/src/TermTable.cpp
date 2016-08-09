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

#include <sstream>

#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Term.h"
#include "TermTable.h"


// TODO: Remove this temporary include. 
#include "BitFunnel/Row.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // TermTable
    //
    //*************************************************************************
    void TermTable::OpenTerm()
    {
        m_start = static_cast<RowIndex>(m_rowIds.size());
    }


    void TermTable::AddRowId(RowId row)
    {
        m_rowIds.push_back(row);
    }


    void TermTable::CloseTerm(Term::Hash hash)
    {
        // Verify that this Term::Hash hasn't been added previously.
        auto it = m_termHashToRows.find(hash);
        if (it != m_termHashToRows.end())
        {
            std::stringstream message;
            message << "TermTable::CloseTerm(): Term::Hash " << hash << " has already been added.";

            RecoverableError error(message.str());
        }

        RowIndex end = static_cast<RowIndex>(m_rowIds.size());
        m_termHashToRows.insert(std::make_pair(hash, std::make_pair(m_start, end)));
    }


    //size_t TermTable::GetTotalRowCount(Rank /*rank*/) const
    //{
    //    throw 0;
    //}

    TermTable::const_iterator TermTable::GetRows(Term term) const
    {
        auto it = m_termHashToRows.find(term.GetRawHash());
        if (it != m_termHashToRows.end())
        {
            return const_iterator(&m_rowIds[(*it).second.first],
                                  &m_rowIds[(*it).second.second]);
        }
        else
        {
            return end();
        }
    }


    TermTable::const_iterator TermTable::end() const
    {
        return const_iterator(nullptr, nullptr);
    }


    //*************************************************************************
    //
    // TermTable::const_iterator
    //
    //*************************************************************************
    TermTable::const_iterator::const_iterator(RowId const * start, RowId const * end)
        : m_current(start),
          m_end(end)
    {
    }


    RowId TermTable::const_iterator::operator*() const
    {
        if (m_current == m_end)
        {
            RecoverableError error("TermTable::const_iterator::operator*: no more entries.");
            throw error;
        }

        return *m_current;
    }

    TermTable::const_iterator& TermTable::const_iterator::operator++()
    {
        if (m_current == m_end)
        {
            RecoverableError error("TermTable::const_iterator::operator++: no more entries.");
            throw error;
        }

        ++m_current;
        return *this;
    }


    // TODO: Remove this temporary StreamId.
    const Term::StreamId c_metaWord = 0;

    // Helper function to create a system internal term. System internal terms
    // are special terms whose Raw hash values by convention are sequential
    // numbers starting at 0. The number of such terms is specified in
    // c_systemRowCount.
    Term CreateSystemInternalTerm(Term::Hash rawHash)
    {
        if (rawHash >= c_systemRowCount)
        {
            RecoverableError error("Hash out of internal row range.");
            throw error;
        }

        const Term term(rawHash,
                        c_metaWord,
                        static_cast<Term::IdfX10>(0));
        return term;
    }



    //*************************************************************************
    //
    // Some random ITermTable static methods.
    // TODO: figure out where these should go.
    //
    //*************************************************************************
    Term ITermTable::GetSoftDeletedTerm()
    {
        static Term softDeletedTerm(CreateSystemInternalTerm(0));
        return softDeletedTerm;
    }


    Term ITermTable::GetMatchAllTerm()
    {
        static Term matchAllTerm(CreateSystemInternalTerm(1));
        return matchAllTerm;
    }


    Term ITermTable::GetMatchNoneTerm()
    {
        static Term matchNoneTerm(CreateSystemInternalTerm(2));
        return matchNoneTerm;
    }
}
