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


#include "BitFunnel/Index/DocumentHandle.h"  // For DocId.
#include "BitFunnel/Index/Factories.h"
#include "DocumentDataSchema.h"

namespace BitFunnel
{
    std::unique_ptr<IDocumentDataSchema> Factories::CreateDocumentDataSchema()
    {
        return std::unique_ptr<IDocumentDataSchema>(new DocumentDataSchema());
    }


    // static const size_t s_bytesPerDocId = sizeof(DocId);

    DocumentDataSchema::DocumentDataSchema()
        : m_variableSizeBlobCount(0)
    {
    }


    VariableSizeBlobId DocumentDataSchema::RegisterVariableSizeBlob()
    {
        return m_variableSizeBlobCount++;
    }


    FixedSizeBlobId DocumentDataSchema::RegisterFixedSizeBlob(unsigned byteCount)
    {
        const FixedSizeBlobId blobId = static_cast<FixedSizeBlobId>(m_fixedSizeBlobSizes.size());

        m_fixedSizeBlobSizes.push_back(byteCount);

        return blobId;
    }


    unsigned DocumentDataSchema::GetVariableSizeBlobCount() const
    {
        return m_variableSizeBlobCount;
    }


    std::vector<unsigned> const & DocumentDataSchema::GetFixedSizeBlobSizes() const
    {
        return m_fixedSizeBlobSizes;
    }
}
