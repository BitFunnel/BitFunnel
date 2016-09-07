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


#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Index/IDocumentCache.h"
#include "IDocumentCacheNode.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // IDocumentCache::const_iterator
    //
    //*************************************************************************
    IDocumentCache::const_iterator::const_iterator(Node const * node)
        : m_current(node)
    {
    }


    bool IDocumentCache::const_iterator::operator!=(
        const_iterator const & other) const
    {
        return m_current != other.m_current;
    }


    IDocumentCache::const_iterator& IDocumentCache::const_iterator::operator++()
    {
        if (m_current == nullptr)
        {
            RecoverableError error("IDocumentCache::const_iterator: attempt to ++ beyond end of collection.");
            throw error;
        }
        else
        {
            m_current = m_current->GetNext();
        }
        return *this;
    }


    IDocumentCache::Entry const IDocumentCache::const_iterator::operator*() const
    {
        return std::pair<IDocument const &, DocId>(
            m_current->GetDocument(), m_current->GetId());
    }
}
