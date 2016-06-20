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

//#include <iosfwd>
#include <memory>                               // std::unique_ptr member.
#include <ostream>                              // TODO: Remove this temporary include.
#include <unordered_map>                        // TODO: Remove this temporary include.
//#include <vector>
//
//#include "BitFunnel/Internal/IShardIndex.h"  // Inherits from IShardIndex.
#include "BitFunnel/NonCopyable.h"
//#include "BitFunnel/RowId.h"                 // Embeds RowId.
//#include "DocTableDescriptor.h"              // Required for embedded std::unique_ptr.
#include "DocumentHandleInternal.h"          // Return value.
//#include "Mutex.h"
//#include "RowTableDescriptor.h"              // Required for embedded std::vector.
#include "Term.h"                               // TODO: Remove this temporary include. Added for unordered_map.
#include "Slice.h"                              // std::unique_ptr template parameter.


namespace BitFunnel
{
    //class IDocumentDataSchema;
    //class IngestionIndex;
    //class ISliceBufferAllocator;
    //class ITermTable;
    class IIngestor;
    class Slice;
    class Term;     // TODO: Remove this temporary declaration.

    //*************************************************************************
    //
    // Shard is an implementation of the IShardIndex interface which represents
    // a partition of the index where documents share common properties. The 
    // Shard maintains a collection of Slices, each of which holds set of 
    // documents. The Shard may add or remove Slices as needed to adjust its 
    // capacity. All Slices in the shard share the same characteristics such 
    // as capacity and size of their memory buffer.
    //
    // Thread safety: all public methods are thread safe.
    //
    //*************************************************************************
    class Shard : private NonCopyable
    {
    public:
        typedef size_t Id;

        // Constructs an empty Shard with no slices. sliceBufferSize must be 
        // sufficient to hold the minimum capacity Slice. The minimum capacity
        // is determined by a value returned by Row::DocumentsInRank0Row(1).
        Shard(IIngestor& ingestor,
              Id id);
        //Shard(IngestionIndex& index, 
        //      ShardId id,
        //      ITermTable const & termTable,
        //      IDocumentDataSchema const & docDataSchema,
        //      ISliceBufferAllocator& sliceBufferAllocator,
        //      size_t sliceBufferSize);

        void TemporaryAddPosting(Term const & term, DocIndex index);
        void TemporaryPrintFrequencies(std::ostream& out);

        //
        // IShardIndex APIs.
        //
        // Returns the Id of the shard.
        //virtual ShardId GetId() const override;

        // Returns capacity of a single Slice in the Shard. All Slices in the 
        // Shard have the same capacity.
        //virtual DocIndex GetSliceCapacity() const override;

        // Returns a vector of slice buffers for this shard.
        // The callers needs to obtain a Token from ITokenManager to protect
        // the pointer to the list of slice buffers, as well as the buffers 
        // themselves.
        //virtual std::vector<void*> const & GetSliceBuffers() const override;

        // Returns the offset of the row in the slice buffer in a shard.
        //virtual ptrdiff_t GetRowOffset(RowId rowId) const override;

        // Returns the offset in the slice buffer where a pointer to the Slice
        // is stored. This is the same offset for all slices in the Shard.
        //virtual ptrdiff_t GetSlicePtrOffset() const override;

        //
        // Shard exclusive members.
        //

        // Allocates storage for a new document. Creates a new slice if 
        // required. Returns a handle to the document storage for the caller
        // to use to populate document's contents. When there is no space in
        // the current slice and no memory available in the 
        // SliceBufferAllocator, this method throws.
        //
        // Implementation:
        // with (m_slicesLock)
        //   DocIndex docIndex;
        //   while (m_activeSlice == nullptr || !m_activeSlice->TryAllocateDocument(docIndex))
        //   {
        //       CreateNewActiveSlice();
        //   }
        //   
        //   return DocumentHandleInternal(m_activeSlice, docIndex);
        DocumentHandleInternal AllocateDocument();

        // Loads a Slice from a previously serialized state and adds it to the
        // list of Slices. As part of deserialization, LoadSlice loads 
        // RowTable/DocTable descriptors from the stream and verifies that it 
        // is compatible with the current descriptors. Uses descriptor's 
        // IsCompatibleWith methods for verification, and if not passed, this 
        // function throws.
        // Design intent is that the serialized files act as a cache, and it
        // is expected that the index may not be able to restore some or all 
        // slices from the cache, and the host will re-ingest the documents 
        // which were not restored.
        //void LoadSlice(std::istream& input);

        // TODO: WriteSlice here or in Slice?

        // Remove slice buffer and its Slice from the list of slices. Throws if
        // slice buffer wasn't found in the list of active slice buffers. 
        // Throws if the slice buffer being removed corresponds to a Slice 
        // which is not fully expired. A slice which is removed, along with the 
        // old copy of the vector of slices, is scheduled for recycling.
        //void RecycleSlice(Slice& slice);

        // Returns the IngestionIndex that owns the Shard.
        IIngestor& GetIndex() const;

        // Returns term table associated with this shard.
        //ITermTable const & GetTermTable() const;

        // Descriptor for RowTables and DocTable.
        //DocTableDescriptor const & GetDocTable() const;
        //RowTableDescriptor const & GetRowTable(Rank) const;

        // Returns the RowId which corresponds to a row used to mark documents
        // as soft-deleted.
        //RowId GetSoftDeletedRowId() const;

        // Allocates memory for the slice buffer. The buffer has the size of
        // m_sliceBufferSize.
        //void* AllocateSliceBuffer();

        // Allocates and loads the contents of the slice buffer from the 
        // stream. The stream has the size of the buffer embedded as the first 
        // element, and the function verifies that it matches the value stored 
        // in buffer m_sliceBufferSize.
        //void* LoadSliceBuffer(std::istream& input);

        // Writes the contents of the slice buffer to the output stream. The 
        // size of the stream is stored in m_sliceBufferSize and is written 
        // before the buffer's data for compatibility checks.
        //void WriteSliceBuffer(void* buffer, std::ostream& output);

        // Releases the slice buffer and returns it to the 
        // ISliceBufferAllocator.
        //void ReleaseSliceBuffer(void* sliceBuffer);

        // Returns the size in bytes of the used capacity in the Shard.
        //unsigned __int64 GetUsedCapacityInBytes() const;

        // Returns the buffer size required to store a single Slice based on 
        // the capacity and schema. If the optional Shard argument is provided, 
        // then it also initializes its DocTable and RowTable descriptors. 
        // The same function combines both actions in order to avoid code 
        // for the two scenarios.
        // DESIGN NOTE: This is made public in order to be used in unit tests.
        //static size_t InitializeDescriptors(Shard* shard,
        //                                    DocIndex sliceCapacity,
        //                                    IDocumentDataSchema const & docDataSchema,
        //                                    ITermTable const & termTable);

        // Calculates the number of documents which can be hosted in a slice 
        // buffer of the given byte size.
        //static DocIndex GetCapacityForByteSize(size_t bufferByteSize,
        //                                       IDocumentDataSchema const & schema,
        //                                       ITermTable const & termTable);

    private:
        // Tries to add a new slice. Throws if no memory in the allocator.
        // Implementation:
        //   std::vector<void*>* newSlices = new std::vector<void*>(m_sliceBuffers);
        //   Slice* newSlice = new Slice(*this);
        //   newSlices.push_back(newSlice->GetBuffer());
        //   swap newSlices and m_sliceBuffers, schedule newSlices for recycling.
        //void CreateNewActiveSlice();

        // Parent IngestionIndex that contains this Shard.
        IIngestor& m_ingestor;

        // Shard's ID.
        Id m_id;

        // TermTable for this shard.
        //ITermTable const & m_termTable;

        // Allocator that provides blocks of memory for Slice buffers.
        //ISliceBufferAllocator& m_sliceBufferAllocator;

        // Row which is used to mark documents as soft deleted.
        // The value of 0 means the document in this column is soft deleted
        // and excluded from matching. Typically this is a private rank 0 row in
        // DDR. During the AddDocument workflow, the bit in this row is set 
        // to 1 as the last step and this effectively makes the document 
        // "serving".
        //const RowId m_softDeletedRowId;

        // Lock protecting operations on the list of slices. Protects the
        // AllocateDocument method.
        // This lock is used in const member functions, as a result, it is
        // declared as mutable.
        //mutable Mutex m_slicesLock;

        // Pointer to the current Slice where documents are being ingested to.
        // Initially set to nullptr. First call to AllocateDocument() will 
        // allocate a new Slice via CreateNewActiveSlice().
        Slice* m_activeSlice;
        std::unique_ptr<Slice> m_slice;

        // Vector of pointers to slice buffers.
        //
        // DESIGN NOTE: We store a pointer to an std::vector here instead of 
        // embedding the vector in order to support lock free vector replacement. 
        // A vector modification is implemented as a copy, followed by an 
        // interlocked exchange of vector pointers. This approach allows query 
        // processing to run lock free at full speed while another thread adds 
        // and removes slices.
        //
        // DESIGN NOTE: We store a vector of void*, instead of Slice* in order 
        // to provide an array of Slice buffer pointers to the matcher.
        //
        // TODO: make this std::vector<void*> const * when a thread safe swap
        // of vectors is implemented.
        //std::vector<void*>* m_sliceBuffers;

        // Capacity of a Slice. All Slices in the shard have the same capacity.
        //const DocIndex m_sliceCapacity;

        // Size of the buffer for storing data for a single Slice. This member 
        // is required for two reasons.
        // 1. Shard needs to convert from slice buffer to Slice* in order to
        //    recycle an empty slice.
        // 2. This value will be passed to ISliceBufferAllocator in order to 
        //    allow handling allocations of buffers of different byte sizes
        //    in future.
        //const size_t m_sliceBufferSize;

        // Descriptors for RowTables and DocTable.
        // DESIGN NOTE: using pointers, rather than embedded instances to avoid 
        // initializer order dependencies in constructor list.
        //std::unique_ptr<DocTableDescriptor> m_docTable;
        //std::vector<RowTableDescriptor> m_rowTables;

        std::unordered_map<Term, size_t, Term::Hasher> m_temporaryFrequencyTable;
    };
}
