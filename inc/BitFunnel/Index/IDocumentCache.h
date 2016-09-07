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

#include <memory>                       // std::unique_ptr parameter.
#include <utility>                      // std::pair in typedef.

#include "BitFunnel/BitFunnelTypes.h"   // DocId parameter.
#include "BitFunnel/IInterface.h"       // Base class.


namespace BitFunnel
{
    class IDocument;

    //*************************************************************************
    //
    // IDocumentCache is an abstract class or interface of classes that
    // represents a collection of IDocuments.
    //
    // The class is intended to capture IDocuments as they are ingested in order
    // to make these IDocuments available to diagnostic query verification code.
    //
    // Thread safety: all methods are thread safe.
    // Because IDocuments can only be added, all iterators are valid in the
    // presence of writer threads.
    //
    //*************************************************************************
    class IDocumentCache : public IInterface
    {
    public:
        // Adds an IDocument to the cache. NOTE that this method transfers
        // ownership of the IDocument. The cache is responsible for destroying
        // the IDocument.
        virtual void Add(std::unique_ptr<IDocument> document,
                         DocId id) = 0;


        class Node;

        typedef std::pair<IDocument const &, DocId> Entry;

        class const_iterator
            : public std::iterator<std::input_iterator_tag, Entry>
        {
        public:
            const_iterator(Node const * node);
            bool operator!=(const_iterator const & other) const;
            const_iterator& operator++();
            Entry const operator*() const;

        private:
            Node const * m_current;
        };


        // These iterators are valid in the presense of writer threads, but
        // they may omit items that are added after begin() returns.
        virtual const_iterator begin() const = 0;
        virtual const_iterator end() const = 0;
    };
}
