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

#include <iosfwd>
#include <memory>                           // For ptrdiff_t.

#include "BitFunnel/Index/DocumentHandle.h"  // For DocId, DocIndex
#include "BitFunnel/Index/IDocumentDataSchema.h"  // For VariableSizeBlobId, FixedSizeBlobId.

namespace NativeJIT
{
    template <typename T> class Node;
}

namespace BitFunnel
{
    //*************************************************************************
    //
    // DocTable is a collection of per-document data items for a slice. An item
    // in the DocTable consists of pointers to variable size blobs along with
    // some fixed size per document data.
    //
    // DocTableDescriptor exposes DocTable operations over a buffer of data.
    // An index will typically have many Slices, but the layout of the slice
    // buffers within a single shard is exactly the same, therefore there is a
    // single instance of DocTableDescriptor which works with multiple slice
    // buffers.
    //
    // Each DocTable entry begins with the 8 bytes of storage for DocId,
    // followed by N descriptors of variable-sized blobs, followed by a block
    // of M bytes of fixed-size storage where
    // N = IDocumentDataSchema::GetVariableSizeBlobCount,
    // M = sum of sizes of the fixed size blobs as returned by
    // IDocumentDataSchema::GetFixedSizeBlobSizes.
    // Each descriptor of the variable size blob contains a pointer to its data
    // and a size. The size is needed during serialization of the DocTable's
    // contents.
    //
    // This is made a helper class instead of a namespace for because of the
    // following benefits:
    // - No need to carry the schema in all calls.
    // - Using a class allows us to cache the number of bytes per item.
    // - Encapsulate the logic of initializing and destroying items, including
    //   freeing the allocated buffers on the heap.
    // - Layout of the DocTable is exactly the same for all Slices in the Shard
    //   and in the index which allows having a single instance of
    //   DocTableDescriptor working over many memory buffers.
    //
    //*************************************************************************
    class DocTableDescriptor
    {
    public:
        // Initializes items in the allocated buffer and sets their pointers for
        // variable sized blobs to nullptr. bufferOffset represents the offset
        // where DocTable data starts in the larger sliceBuffer. The address of
        // the sliceBuffer is passed to other methods of this class and there
        // has to be enough space starting at sliceBuffer + bufferOffset, as
        // determined by a call to GetBufferSize().
        DocTableDescriptor(DocIndex capacity,
                           IDocumentDataSchema const & schema,
                           ptrdiff_t bufferOffset);

        // Copy constructor from another DocTableDescriptor. Required so that a
        // Slice can create a cached copy of the DocTableDescriptor from Shard.
        DocTableDescriptor(DocTableDescriptor const & other);

        // Initializes the DocTable in the block of memory at sliceBuffer +
        // bufferOffset, where bufferOffset was the value passed to the
        // constructor. This block must be large enough to hold the DocTable, as
        // determined by GetBufferSize().
        void Initialize(void* sliceBuffer) const;

        // Loads the contents of the variable size blobs from the stream.
        // DESIGN NOTE: Fixed size blobs are part of the slice buffer and are
        // loaded as part of loading the whole slice buffer.
        void LoadVariableSizeBlobs(void* sliceBuffer, std::istream& input) const;

        // Writes the contents of the variable size blobs to the stream.
        // DESIGN NOTE: Fixed size blobs are part of the slice buffer and are
        // written out along with the whole slice buffer.
        void WriteVariableSizeBlobs(void* sliceBuffer, std::ostream& output) const;

        // Releases memory held by the variable sized blobs.
        void Cleanup(void* sliceBuffer) const;

        // Allocates buffer for variable sized blob of per-document data.
        // Throws if this blob had previously been allocated.
        void* AllocateVariableSizeBlob(void* sliceBuffer,
                                       DocIndex index,
                                       VariableSizeBlobId blob,
                                       size_t byteCount) const;

        // Returns a pointer to a variable sized blob of per-document data.
        // Returns null if this blob has not been previously allocated.
        void* GetVariableSizeBlob(void* sliceBuffer,
                                  DocIndex index,
                                  VariableSizeBlobId blob) const;

        // Returns a pointer to a variable sized blob of per-document data.
        void* GetFixedSizeBlob(void* sliceBuffer,
                               DocIndex index,
                               FixedSizeBlobId blob) const;

        // Returns the document's unique identifier.
        DocId GetDocId(void* sliceBuffer, DocIndex index) const;

        // Stores the document's unique identifier.
        void SetDocId(void* sliceBuffer, DocIndex index, DocId id) const;

        //
        // NaviteJIT methods.
        //
        NativeJIT::Node<void*>
            JITGetVariableSizeBlob(NativeJIT::Node<void*> sliceBuffer,
                                   NativeJIT::Node<DocIndex> index,
                                   VariableSizeBlobId blob);

        NativeJIT::Node<void*>
            JITGetFixedSizeBlob(NativeJIT::Node<void*>,
                                NativeJIT::Node<DocIndex>,
                                FixedSizeBlobId blob);

        // Returns true if the given DocTableDescriptor is data-compatible with
        // this instance. Used when loading Slices from the stream.
        // TODO: confirm our compatibility policy.
        bool IsCompatibleWith(DocTableDescriptor const & other) const;

        // Represents a descriptor for a variable size blob which contains the
        // address of the blob and its size.
        // DESIGN NOTE: size is needed during DocTable contents serialization.
        // DESIGN NOTE: this is made public since it is used in a static
        // GetBufferSize method.
#pragma pack(push, 1)
        // maximally compact object, size over perf.
        struct VariableSizeBlob
        {
            void* m_data;
            uint32_t m_size;
        };

        // VariableSizeBlob consits of 8 bytes of pointer to its contents and
        // 4 bytes of the size. Enforcing the minimal size possible.
        static_assert(sizeof(VariableSizeBlob) == 12, "VariableSizeBlob must be 12 bytes");
#pragma pack(pop)

        // Returns the size of the buffer in bytes that is required to host a
        // DocTable with a particular capacity and IDocumentDataSchema
        // (excluding data allocated for variable size blobs which is allocated
        // from the heap). This will assist the class that manages the buffer
        // with allocation of proper sized buffers.
        static size_t GetBufferSize(DocIndex capacity,
                                    IDocumentDataSchema const & schema);

    private:

        // Declare but don't implement. This is required for a std::vector to
        // use a copy constructor instead of assignment operator.
        DocTableDescriptor& operator=(DocTableDescriptor const & other);

        // Returns a reference to a pointer which stores the blob.
        VariableSizeBlob& GetVariableBlobRef(void* sliceBuffer,
                                             DocIndex index,
                                             VariableSizeBlobId blob) const;

        // Returns a reference to the blob data which contains DocId.
        DocId& GetDocIdBlobData(void* sliceBuffer, DocIndex index) const;

        // Returns a pointer to the beginning of the item with the given
        // index position.
        char* GetItem(void* sliceBuffer, DocIndex index) const;

        // Offset in the slice buffer at which DocTable starts.
        const ptrdiff_t m_bufferOffset;

        // Capacity of the DocTable.
        const DocIndex m_capacity;

        // DocTable caches properties of the IDocumentDataSchema to make itself
        // self-contained and more performant.
        unsigned m_variableSizeBlobCount;
        std::vector<unsigned> m_fixedSizeBlobOffsets;

        // The number of bytes per single entry in the DocTable. Consists of
        // bytes required to store pointers to variable size blobs and fixed
        // size storage.
        const size_t m_bytesPerItem;
    };

}
