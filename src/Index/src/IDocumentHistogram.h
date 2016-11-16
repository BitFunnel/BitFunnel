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

#include <ostream>      // std::ostream parameter.


namespace BitFunnel
{
    //*************************************************************************
    //
    // IDocumentHistogram is an abstract base class or interface for classes
    // that represent a histogram of document count and document body length
    // sum for each quantity of postings associated with a document. The
    // histogram can be accessed as an ordered set of entries containing
    // posting count, document count, and body length sum. These entries are
    // ordered by increasing posting count, but posting counts are not required
    // to be consecutive.
    //
    //*************************************************************************
    class IDocumentHistogram
    {
    public:
        virtual ~IDocumentHistogram() {};

        // Persists the contents of the histogram to a stream. Typically
        // classes that implement IDocumentHistogram will provide a
        // constructor that initializes the histogram from a stream.
        virtual void Write(std::ostream& output) const = 0;

        // Returns the number of entries within the histogram.
        virtual size_t GetEntryCount() const = 0;

        // Returns the sum of the document counts across all of the entries
        // in the histogram. This is equal to the number of documents in the
        // corpus used to create the histogram.
        virtual double GetTotalDocumentCount() const = 0;

        // Returns the posting count associated with a specific entry.
        virtual unsigned GetPostingCount(size_t index) const = 0;

        // Returns the document count associated with a specific entry.
        // This is the number of documents in the corpus that have posting
        // counts equal to GetPostingCount(index).
        virtual double GetDocumentCount(size_t index) const = 0;

        // Returns the average document body length associated with a specific
        // entry. This average is over all documents in the corpus that have
        // posting counts equal to GetPostingCount(index).
        virtual double GetAverageBodyLength(size_t index) const = 0;

        // Adds a new entry to the histogram. The posting count must be
        // strictly greater than the largest posting count already in the
        // histogram.
        virtual void AddEntry(size_t postingCount,
                              double documentCount,
                              double averageBodyLength) = 0;
    };
}
