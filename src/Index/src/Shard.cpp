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

#include <iostream>     // TODO: Remove this temporary header.

#include "BitFunnel/Exceptions.h"
#include "Shard.h"
#include "Term.h"       // TODO: Remove this temporary include.


namespace BitFunnel
{
    Shard::Shard(IIngestor& ingestor, Id id)
        : m_ingestor(ingestor),
          m_id(id),
          m_slice(new Slice(*this))
    {
        m_activeSlice = m_slice.get();
    }


    void Shard::TemporaryAddPosting(Term const & term, DocIndex index)
    {
        std::cout << "  " << index << ": ";
        term.Print(std::cout);
        std::cout << std::endl;

        {
            // TODO: Remove this lock once it is incorporated into the frequency table class.
            std::lock_guard<std::mutex> lock(m_temporaryFrequencyTableMutex);
            m_temporaryFrequencyTable[term]++;
        }
    }


    void Shard::TemporaryPrintFrequencies(std::ostream& out)
    {
        out << "Term frequency table for shard " << m_id << ":" << std::endl;
        for (auto it = m_temporaryFrequencyTable.begin(); it != m_temporaryFrequencyTable.end(); ++it)
        {
            out << "  ";
            it->first.Print(out);
            out << ": "
                << it->second
                << std::endl;
        }
        out << std::endl;
    }


    DocumentHandleInternal Shard::AllocateDocument()
    {
        DocIndex index;
        if (!m_activeSlice->TryAllocateDocument(index))
        {
            throw FatalError("In this temporary code, TryAllocateDocument() should always succeed.");
        }
        return DocumentHandleInternal(m_activeSlice, index);
    }


    IIngestor& Shard::GetIndex() const
    {
        return m_ingestor;
    }
}
