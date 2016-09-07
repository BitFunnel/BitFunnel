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

#include "BitFunnel/BitFunnelTypes.h"       // DocId parameter.
#include "BitFunnel/Index/IDocument.h"      // IDocument template parameter.
#include "BitFunnel/Index/IDocumentCache.h" // Containing class.


namespace BitFunnel
{
    //*************************************************************************
    //
    // IDocumentCache::Node
    //
    // Linked list node used by IDocumentCache.
    //
    // DESIGN NOTE: Using a linked list instead of a vector to allow for
    // iterators to remain valid in the presense of writer threads. An
    // IDocumentCache::const_iterator just holds a Node const *. New nodes
    // can be added at any time, but their addition cannot impact existing
    // nodes.
    //
    //*************************************************************************
    class IDocumentCache::Node
    {
    public:
        Node(std::unique_ptr<IDocument> document,
             DocId id,
             Node const * next)
          : m_document(std::move(document)),
            m_id(id),
            m_next(next)
        {
        }

        IDocument const & GetDocument() const
        {
            return *m_document;
        }

        DocId const & GetId() const
        {
            return m_id;
        }

        Node const * GetNext() const
        {
            return m_next;
        }

    private:
        std::unique_ptr<IDocument const> m_document;
        DocId m_id;
        Node const * m_next;
    };
}
