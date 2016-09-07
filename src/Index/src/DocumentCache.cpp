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


#include "DocumentCache.h"
#include "IDocumentCacheNode.h"


namespace BitFunnel
{
    DocumentCache::DocumentCache()
        : m_head(nullptr)
    {
    }


    DocumentCache::~DocumentCache()
    {
        // Technically speaking we shouldn't have to take the lock here since
        // other threads shouldn't be calling us as we're being destructed.
        std::lock_guard<std::mutex> lock(m_lock);
        while (m_head.load() != nullptr)
        {
            Node const * temp = m_head;
            m_head = m_head.load()->GetNext();
            delete temp;
        }
    }


    void DocumentCache::Add(std::unique_ptr<IDocument> document,
                            DocId id)
    {
        // Allocate space for new node before taking lock.
        char * buffer = new char[sizeof(Node)];

        // Lock protects m_head from other writers.
        std::lock_guard<std::mutex> lock(m_lock);
        Node const * head = new (buffer) Node(std::move(document), id, m_head);
        m_head = head;
    }


    IDocumentCache::const_iterator DocumentCache::begin() const
    {
        // Readers don't need a lock because m_head is std::atomic. They will
        // see either the old head or the new head.
        return const_iterator(m_head);
    }


    IDocumentCache::const_iterator DocumentCache::end() const
    {
        return const_iterator(nullptr);
    }
}
