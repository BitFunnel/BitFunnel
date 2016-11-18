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

#include <iosfwd>   // std::istream & parameter.
#include <utility>  // std::pair template parameter.
#include <vector>   // std::vector embedded.

#include "BitFunnel/Index/IDocumentHistogram.h"     // Base class.


namespace BitFunnel
{
    class DocumentHistogram : public IDocumentHistogram
    {
    public:
        DocumentHistogram(std::istream & input);

        //
        // IDocumentHistogram methods.
        //

        // Returns the number of entries within the histogram.
        virtual size_t GetEntryCount() const override;

        // Returns the sum of the document counts across all of the entries
        // in the histogram. This is equal to the number of documents in the
        // corpus used to create the histogram.
        virtual double GetTotalDocumentCount() const override;

        // Returns the posting count associated with a specific entry.
        virtual size_t GetPostingCount(size_t index) const override;

        // Returns the document count associated with a specific entry.
        // This is the number of documents in the corpus that have posting
        // counts equal to GetPostingCount(index).
        virtual double GetDocumentCount(size_t index) const override;

    private:
        size_t m_documentCount;
        std::vector<std::pair<size_t, size_t>> m_entries;
    };
}
