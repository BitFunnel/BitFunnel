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


#include <cstring>
#include <memory>
#include <stdlib.h>

#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Utilities/StreamUtilities.h"
#include "DocTableDescriptor.h"
#include "Rounding.h"


namespace BitFunnel
{
    // Calculates the total byte size of the fixed size storage.
    unsigned GetFixedSizeTotalByteCount(IDocumentDataSchema const & schema)
    {
        // DocId is added to the fixed size storage.
        unsigned totalSize = sizeof(DocId);
        std::vector<unsigned> const & fixedSizeBlobSizes =
            schema.GetFixedSizeBlobSizes();
        for (const auto size : fixedSizeBlobSizes)
        {
            totalSize += size;
        }

        return totalSize;
    }


    // Returns the size in bytes that each record of the DocTable occupies
    // (excluding data allocated for variable size blobs which is allocated
    // from the heap).
    size_t GetItemByteCount(IDocumentDataSchema const & schema)
    {
        const unsigned fixedSizeTotalByteCount =
            GetFixedSizeTotalByteCount(schema);

        // A single item consists of pointers to variable size blobs and the
        // fixed sized data.
        unsigned bufferSize =
            sizeof(DocTableDescriptor::VariableSizeBlob) *
            schema.GetVariableSizeBlobCount() +
            fixedSizeTotalByteCount;

        // Round up to the closest power of 2 to make sure no two DocInfos span
        // cache-line.
        return RoundUpPowerOf2(bufferSize);
    }


    // Creates a vector of fixed size blob offsets from their sizes.
    std::vector<unsigned>
        CreateFixedSizeBlobOffsets(IDocumentDataSchema const & schema)
    {
        // DocId is placed first in the fixed size storage.
        unsigned currentOffset = sizeof(DocId);

        // Skip variable size blobs.
        currentOffset += schema.GetVariableSizeBlobCount() *
            sizeof(DocTableDescriptor::VariableSizeBlob);

        std::vector<unsigned> const & fixedSizeBlobSizes =
            schema.GetFixedSizeBlobSizes();

        std::vector<unsigned> offsets;
        offsets.reserve(fixedSizeBlobSizes.size());
        for (const auto size : fixedSizeBlobSizes)
        {
            offsets.push_back(currentOffset);
            currentOffset += size;
        }

        return offsets;
    }


    // Returns the size in bytes that each record of the DocTable occupies
    // (excluding data allocated for variable size blobs which is allocated from
    // the heap).
    /* static */
    size_t DocTableDescriptor::GetBufferSize(DocIndex capacity,
                                             IDocumentDataSchema const & schema)
    {
        const size_t bytesPerItem = GetItemByteCount(schema);

        return bytesPerItem * capacity;
    }


    DocTableDescriptor::DocTableDescriptor(DocIndex capacity,
                                           IDocumentDataSchema const & schema,
                                           ptrdiff_t bufferOffset)
        : m_bufferOffset(bufferOffset),
          m_capacity(capacity),
          m_variableSizeBlobCount(schema.GetVariableSizeBlobCount()),
          m_fixedSizeBlobOffsets(CreateFixedSizeBlobOffsets(schema)),
          m_bytesPerItem(GetItemByteCount(schema))
    {
        // Make sure offset of the DocTable is properly aligned.
        // LogAssertB((bufferOffset % c_docTableByteAlignment) == 0,
        //           "DocTableDescriptor bufferOffset not aligned.");
    }


    DocTableDescriptor::DocTableDescriptor(DocTableDescriptor const & other)
        : m_bufferOffset(other.m_bufferOffset),
          m_capacity(other.m_capacity),
          m_variableSizeBlobCount(other.m_variableSizeBlobCount),
          m_fixedSizeBlobOffsets(other.m_fixedSizeBlobOffsets),
          m_bytesPerItem(other.m_bytesPerItem)

    {
    }


    void DocTableDescriptor::Initialize(void* sliceBuffer) const
    {
        char* const buffer = reinterpret_cast<char*>(sliceBuffer) +
            m_bufferOffset;
        memset(buffer, 0, m_bytesPerItem * m_capacity);
    }


    void DocTableDescriptor::
        LoadVariableSizeBlobs(void* sliceBuffer, std::istream& input) const
    {
        if (m_variableSizeBlobCount > 0)
        {
            for (DocIndex i = 0; i < m_capacity; ++i)
            {
                for (unsigned blob = 0; blob < m_variableSizeBlobCount; ++blob)
                {
                    VariableSizeBlob& blobData =
                        GetVariableBlobRef(sliceBuffer, i, blob);
                    blobData.m_size =
                        StreamUtilities::ReadField<uint32_t>(input);

                    if (blobData.m_size > 0)
                    {
                        blobData.m_data = malloc(blobData.m_size);
                        StreamUtilities::ReadBytes(input,
                                                   blobData.m_data,
                                                   blobData.m_size);
                    }
                    else
                    {
                        blobData.m_data = nullptr;
                    }
                }
            }
        }
    }


    void DocTableDescriptor::
        WriteVariableSizeBlobs(void* sliceBuffer, std::ostream& output) const
    {
        if (m_variableSizeBlobCount > 0)
        {
            for (DocIndex i = 0; i < m_capacity; ++i)
            {
                for (unsigned blob = 0; blob < m_variableSizeBlobCount; ++blob)
                {
                    VariableSizeBlob const & blobData =
                        GetVariableBlobRef(sliceBuffer, i, blob);

                    StreamUtilities::WriteField<uint32_t>(output,
                                                          blobData.m_size);
                    if (blobData.m_size > 0)
                    {
                        StreamUtilities::
                            WriteBytes(output,
                                       reinterpret_cast<char*>(blobData.m_data),
                                       blobData.m_size);
                    }
                }
            }
        }
    }


    void DocTableDescriptor::Cleanup(void* sliceBuffer) const
    {
        if (m_variableSizeBlobCount > 0)
        {
            for (DocIndex i = 0; i < m_capacity; ++i)
            {
                for (unsigned blob = 0; blob < m_variableSizeBlobCount; ++blob)
                {
                    VariableSizeBlob& blobData =
                        GetVariableBlobRef(sliceBuffer, i, blob);

                    if (blobData.m_data != nullptr)
                    {
                        free(blobData.m_data);
                        blobData.m_data = nullptr;
                        blobData.m_size = 0;
                    }
                }
            }
        }
    }


    DocId DocTableDescriptor::GetDocId(void* sliceBuffer, DocIndex index) const
    {
        void* item = GetItem(sliceBuffer, index);
        return *reinterpret_cast<DocId*>(item);
    }


    void DocTableDescriptor::SetDocId(void* sliceBuffer,
                                      DocIndex index,
                                      DocId id) const
    {
        void* item = GetItem(sliceBuffer, index);
        *reinterpret_cast<DocId*>(item) = id;
    }


    void* DocTableDescriptor::AllocateVariableSizeBlob(void* sliceBuffer,
                                                       DocIndex index,
                                                       VariableSizeBlobId blob,
                                                       size_t byteCount) const
    {
        VariableSizeBlob& blobPtr =
            GetVariableBlobRef(sliceBuffer, index, blob);

        // Make sure it hasn't been allocated before.
        if (blobPtr.m_data != nullptr)
        {
            throw FatalError("Blob has already been allocated");
        }

        blobPtr.m_data = malloc(byteCount);
        blobPtr.m_size = static_cast<uint32_t>(byteCount);
        return blobPtr.m_data;
    }


    void* DocTableDescriptor::GetVariableSizeBlob(void* sliceBuffer,
                                                  DocIndex index,
                                                  VariableSizeBlobId blob) const
    {
        return GetVariableBlobRef(sliceBuffer, index, blob).m_data;
    }


    void* DocTableDescriptor::GetFixedSizeBlob(void* sliceBuffer,
                                               DocIndex index,
                                               FixedSizeBlobId blob) const
    {
        void* item = GetItem(sliceBuffer, index) + m_fixedSizeBlobOffsets[blob];

        return item;
    }


    DocTableDescriptor::VariableSizeBlob&
    DocTableDescriptor::GetVariableBlobRef(void* sliceBuffer,
                                           DocIndex index,
                                           VariableSizeBlobId blob) const
    {
        void* blobPtr = GetItem(sliceBuffer, index) +
            sizeof(DocId) +
            blob * sizeof(VariableSizeBlob);

        return *reinterpret_cast<VariableSizeBlob*>(blobPtr);
    }


    char* DocTableDescriptor::GetItem(void* sliceBuffer, DocIndex index) const
    {
        return reinterpret_cast<char*>(sliceBuffer) +
            m_bufferOffset +
            m_bytesPerItem * index;
    }
}
