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

#include <vector>

#include "BitFunnel/Index/IDocumentDataSchema.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // DocumentDataSchema is an implementation of IDocumentDataSchema which
    // gives out sequential numbers for blob IDs.
    //
    // Thread safety: thread safe for multiple readers. Not thread safe for
    // multiple writers.
    //
    //*************************************************************************
    class DocumentDataSchema : public IDocumentDataSchema
    {
    public:
        DocumentDataSchema();

        //
        // IDocumentDataSchema API.
        //
        virtual VariableSizeBlobId RegisterVariableSizeBlob() override;
        virtual FixedSizeBlobId RegisterFixedSizeBlob(unsigned byteCount) override;
        virtual unsigned GetVariableSizeBlobCount() const override;
        virtual std::vector<unsigned> const & GetFixedSizeBlobSizes() const override;

    private:

        // The number of variable sized blobs.
        unsigned m_variableSizeBlobCount;

        // Sizes of the fixed-size per document data added by different
        // constituants of the document ingestion. FixedSizeBlobId acts as an
        // index into this array.
        std::vector<unsigned> m_fixedSizeBlobSizes;
    };
}
