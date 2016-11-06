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

#include "BitFunnel/Chunks/DocumentFilters.h"
#include "BitFunnel/Index/IDocument.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // CompositeFilter
    //
    //*************************************************************************
    void CompositeFilter::AddFilter(std::unique_ptr<IDocumentFilter> filter)
    {
        m_filters.push_back(std::move(filter));
    }


    bool CompositeFilter::KeepDocument(IDocument const & document)
    {
        for (auto & filter : m_filters)
        {
            if (!filter->KeepDocument(document))
            {
                return false;
            }
        }

        return true;
    }


    //*************************************************************************
    //
    // RandomDocumentFilter
    //
    //*************************************************************************
    RandomDocumentFilter::RandomDocumentFilter(unsigned seed, double fraction)
      : m_engine(seed, 0.0, 1.0),
        m_fraction(fraction)
    {
    }


    bool RandomDocumentFilter::KeepDocument(IDocument const & /*document*/)
    {
        return m_engine() < m_fraction;
    }


    //*************************************************************************
    //
    // PostingCountFilter
    //
    //*************************************************************************
    PostingCountFilter::PostingCountFilter(size_t minCount, size_t maxCount)
      : m_minCount(minCount),
        m_maxCount(maxCount)
    {
    }


    bool PostingCountFilter::KeepDocument(IDocument const & document)
    {
        const size_t count = document.GetPostingCount();
        return count >= m_minCount && count <= m_maxCount;
    }


    //*************************************************************************
    //
    // DocumentCountFilter
    //
    //*************************************************************************
    DocumentCountFilter::DocumentCountFilter(size_t documentCount)
      : m_maxDocumentCount(documentCount),
        m_documentCount(0)
    {
    }


    bool DocumentCountFilter::KeepDocument(IDocument const & /*document*/)
    {
        if (m_documentCount < m_maxDocumentCount)
        {
            ++m_documentCount;
            return true;
        }
        return false;
    }


    //*************************************************************************
    //
    // NopFilter
    //
    //*************************************************************************
    bool NopFilter::KeepDocument(IDocument const & /*document*/)
    {
        return true;
    }
}
