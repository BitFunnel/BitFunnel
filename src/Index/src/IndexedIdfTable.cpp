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

#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Utilities/StreamUtilities.h"
#include "IndexedIdfTable.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // Factory methods.
    //
    //*************************************************************************

    std::unique_ptr<IIndexedIdfTable> Factories::CreateIndexedIdfTable()
    {
        return std::unique_ptr<IIndexedIdfTable>(
            new IndexedIdfTable());
    }


    std::unique_ptr<IIndexedIdfTable>
        Factories::CreateIndexedIdfTable(std::istream & input,
                                         Term::IdfX10 defaultIdf)
    {
        return std::unique_ptr<IIndexedIdfTable>(
            new IndexedIdfTable(input, defaultIdf));
    }


    //*************************************************************************
    //
    // IndexedIdfTable
    //
    //*************************************************************************

    // TODO: Proper implementation or remove.
    IndexedIdfTable::IndexedIdfTable()
        : m_defaultIdf(60)
    {
    }


    IndexedIdfTable::IndexedIdfTable(std::istream& input,
                                     Term::IdfX10 defaultIdf)
        : m_defaultIdf(defaultIdf)
    {
        // TODO: Should defaultIdf be part of the file?

        size_t count = StreamUtilities::ReadField<size_t>(input);

        for (size_t i = 0; i < count; ++i)
        {
            const Term::Hash hash(StreamUtilities::ReadField<Term::Hash>(input));
            const Term::IdfX10 idf(StreamUtilities::ReadField<Term::IdfX10>(input));
            m_terms.insert(std::make_pair(hash, idf));
        }
    }


    void IndexedIdfTable::WriteHeader(std::ostream& output, size_t entryCount)
    {
        // TODO: Use FileHeader and version.
        StreamUtilities::WriteField<size_t>(output, entryCount);
    }


    void IndexedIdfTable::WriteEntry(std::ostream& output, Term::Hash hash, Term::IdfX10 idf)
    {
        StreamUtilities::WriteField<Term::Hash>(output, hash);
        StreamUtilities::WriteField<Term::IdfX10>(output, idf);
    }


    Term::IdfX10 IndexedIdfTable::GetIdf(Term::Hash hash) const
    {
        auto it = m_terms.find(hash);
        if (it != m_terms.end())
        {
            return (*it).second;
        }
        else
        {
            return m_defaultIdf;
        }
    }
}
