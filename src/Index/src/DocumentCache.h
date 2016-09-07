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

#include <atomic>                           // std::atomic embedded.
#include <mutex>                            // std::mutex embedded.

#include "BitFunnel/Index/IDocument.h"      // IDocument template parameter.
#include "BitFunnel/Index/IDocumentCache.h" // Base class.


namespace BitFunnel
{
    //*************************************************************************
    //
    // DocumentCache
    //
    //*************************************************************************
    class DocumentCache : public IDocumentCache
    {
    public:
        DocumentCache();
        ~DocumentCache();

        virtual void Add(std::unique_ptr<IDocument> document,
                         DocId id) override;

        virtual const_iterator begin() const override;
        virtual const_iterator end() const override;

    private:
        // m_lock protects m_head from multiple writers.
        std::mutex m_lock;

        // m_head is atomic to support reading in the presenxe of writers.
        std::atomic<Node const *> m_head;
    };
}
