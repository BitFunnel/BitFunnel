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
    class CompositeFilter : public IDocumentFilter
    {
    public:
        void AddFilter(std::unique_ptr<IDocumentFilter> filter);

        virtual bool KeepDocument(IDocument const & document) override;

    private:
        std::vector<std::unique_ptr<IDocumentFilter>> m_filters;
    };


    class RandomDocumentFilter : public IDocumentFilter
    {
    public:
        RandomDocumentFilter(unsigned seed, double fraction);

        virtual bool KeepDocument(IDocument const & document) override;

    private:
        RandomReal<double> m_engine;
        double m_fraction;
    };


    class PostingCountFilter : public IDocumentFilter
    {
    public:
        PostingCountFilter(size_t minCount, size_t maxCount);

        virtual bool KeepDocument(IDocument const & document) override;

    private:
        size_t m_minCount;
        size_t m_maxCount;
    };


    class DocumentCountFilter : public IDocumentFilter
    {
    public:
        DocumentCountFilter(size_t documentCount);

        virtual bool KeepDocument(IDocument const & document) override;

    private:
        const size_t m_maxDocumentCount;
        size_t m_documentCount;
    };


    class NopFilter : public IDocumentFilter
    {
    public:
        virtual bool KeepDocument(IDocument const & document) override;
    };
}
