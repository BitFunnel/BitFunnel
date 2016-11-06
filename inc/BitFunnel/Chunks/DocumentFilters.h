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

#include "BitFunnel/Utilities/Random.h"         // RandomReal embedded.
#include "BitFunnel/Chunks/IChunkProcessor.h"   // Base class.


namespace BitFunnel
{
    //*************************************************************************
    //
    // CompositeFilter
    //
    // An IDocumentFilter that applies a sequence of IDocumentFilters.
    //
    //*************************************************************************
    class CompositeFilter : public IDocumentFilter
    {
    public:
        // Adds a filter to the sequence of filters to be applied.
        // Filters are applied in the order they are added.
        void AddFilter(std::unique_ptr<IDocumentFilter> filter);

        virtual bool KeepDocument(IDocument const & document) override;

    private:
        std::vector<std::unique_ptr<IDocumentFilter>> m_filters;
    };


    //*************************************************************************
    //
    // RandomDocumentFilter
    //
    // Keeps a document with probability equal to the fraction passed to the
    // constructor. Constructor takes a random number generator seed to allow
    // for reproducibility from run to run.
    //
    //*************************************************************************
    class RandomDocumentFilter : public IDocumentFilter
    {
    public:
        RandomDocumentFilter(double fraction, unsigned seed = 12345);

        virtual bool KeepDocument(IDocument const & document) override;

    private:
        const double m_fraction;
        RandomReal<double> m_engine;
    };


    //*************************************************************************
    //
    // PostingCountFilter
    //
    // IDocumentFilter that keeps documents with posting counts in a specified
    // range.
    //
    //*************************************************************************
    class PostingCountFilter : public IDocumentFilter
    {
    public:
        // Constructs a filter that accept documents with posting counts in
        // the range [minCount, maxCount].
        PostingCountFilter(size_t minCount, size_t maxCount);

        virtual bool KeepDocument(IDocument const & document) override;

    private:
        const size_t m_minCount;
        const size_t m_maxCount;
    };


    //*************************************************************************
    //
    // DocumentCountFilter
    //
    // IDocumentFilter that keeps the first n documents.
    //
    //*************************************************************************
    class DocumentCountFilter : public IDocumentFilter
    {
    public:
        // Constructs a filter that accepts the first `documentCount` documents
        // passed to KeepDocument(). NOTE: this filter should be the last in a
        // chain of filters to ensure that all counted documents are actually
        // kept. For example, applying
        //      RandomDocumentFilter
        // before
        //      DocumentCountFilter
        // will ensure that the `documentCount` candidates are kept. If the
        // filters are applied in the reverse order, `documentCount` candidates
        // will be passed to the `RandomDocumentFilter` which will likely
        // reject some.
        DocumentCountFilter(size_t documentCount);

        virtual bool KeepDocument(IDocument const & document) override;

    private:
        const size_t m_maxDocumentCount;

        // Tracks the number of documents kept so far.
        size_t m_documentCount;
    };


    //*************************************************************************
    //
    // NopFilter
    //
    // IDocumentFilter that keeps the all documents.
    //
    //*************************************************************************
    class NopFilter : public IDocumentFilter
    {
    public:
        virtual bool KeepDocument(IDocument const & document) override;
    };
}
