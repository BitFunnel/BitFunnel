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

#include <memory>                           // std::unique_ptr return value.

#include "BitFunnel/BitFunnelTypes.h"       // DocId parameter.
#include "BitFunnel/Index/IDocument.h"      // Base class.


namespace BitFunnel
{
    class IConfiguration;

    //*************************************************************************
    //
    // PrimeFactorsDocument
    //
    //*************************************************************************
    std::unique_ptr<IDocument>
        CreatePrimeFactorsDocument(IConfiguration const & config, DocId id);

    //class PrimeFactorsDocument : public IDocument
    //{
    //public:
    //    // Constructs a document containing terms corresponding to the prime
    //    // factors of the supplied DocId. The document will also contain a term
    //    // corresponding to 1 and the DocId.
    //    //
    //    // The current implementation does not provide support for phrases.
    //    PrimeFactorsDocument(DocId id);

    //    // Returns the number of postings this document will contribute
    //    // to the index. This method is used to determine which shard
    //    // will hold the document.
    //    virtual size_t GetPostingCount() const override;

    //    // Returns the number of bytes of the source representation of this
    //    // document. Used to compute ingestion rate (bytes/second) statistic.
    //    //
    //    // Since this document wasn't constructed from text, the byte size is
    //    // computes as the length of a string containing a comma-separated
    //    // list of the text representation of each factor. For example, the
    //    // document with DocId=100 would be modeled as the string,
    //    //     "1,2,2,5,5"
    //    // so its source byte size would be 9.
    //    virtual size_t GetSourceByteSize() const override;

    //    // Ingests the contents of this document into the index at via
    //    // the supplied DocumentHandle.
    //    virtual void Ingest(DocumentHandle handle) const override;

    //    // Returns true iff the document contains a specific term.
    //    virtual bool Contains(Term & term) const override;


    //    // NOT IMPLEMENTED. Throws BitFunnel::NotImplemented.
    //    virtual void OpenStream(Term::StreamId id) override;

    //    // NOT IMPLEMENTED. Throws BitFunnel::NotImplemented.
    //    virtual void AddTerm(char const * term) override;

    //    // NOT IMPLEMENTED. Throws BitFunnel::NotImplemented.
    //    virtual void CloseStream() override;

    //    // NOT IMPLEMENTED. Throws BitFunnel::NotImplemented.
    //    virtual void CloseDocument(size_t sourceByteSize) override;

    //private:
    //};
}
