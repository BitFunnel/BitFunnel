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

#include "BitFunnel/Index/DocumentHandle.h"     // DocumentHandle parameter.
#include "BitFunnel/IInterface.h"               // Inherits from IInterface.
#include "BitFunnel/Term.h"                     // Term::StreamId parameter.


namespace BitFunnel
{
    class IDocument : public IInterface
    {
    public:
        // Returns the number of postings this document will contribute
        // to the index. This method is used to determine which shard
        // will hold the document.
        virtual size_t GetPostingCount() const = 0;

        // Ingests the contents of this document into the index at via
        // the supplied DocumentHandle.
        virtual void Ingest(DocumentHandle handle) const = 0;


        // Opens a named stream for term additions. Subsequent calls to
        // AddTerm() will add terms to this stream.
        virtual void OpenStream(Term::StreamId id) = 0;

        // Adds a term to the currently opened stream.
        virtual void AddTerm(char const * term) = 0;

        // Closes the current stream.
        virtual void CloseStream() = 0;
    };
}
