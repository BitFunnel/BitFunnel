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

#include <limits>

#include "BitFunnel/BitFunnelTypes.h"   // For DocIndex, DocId.
#include "BitFunnel/Index/IDocumentDataSchema.h"  // VariableSizeBlobId and FixedSizeBlobId are parameters.
#include "BitFunnel/Index/IFactSet.h"  // FactHandle is a parameter.
#include "BitFunnel/RowId.h"            // RowId parameter.


namespace BitFunnel
{
    class Slice;
    class Term;

    // Represent the value that the default constructor assigns to the instances
    // of DocumentHandle.
    static const DocIndex c_invalidDocIndex =
        std::numeric_limits<size_t>::max();


    //*************************************************************************
    //
    // Represents a handle to the document that is being ingested. Implementators
    // of IDocument will use this class to set postings, assert facts, or get
    // access to the variable size or fixed size document blobs in DocTable.
    //
    // DESIGN NOTE: This class is meant to be passed around by copy rather
    // than a reference.
    //
    //*************************************************************************
    class DocumentHandle
    {
    public:
        // Allocates a block of memory for the variable-sized blob associated
        // with a given VariableSizeBlobId. The VariableSizeBlobId must have
        // been previously assigned by a call to
        //    IDocumentDataSchema::RegisterVariableSizeBlob().
        //
        // The block will consist of byteSize bytes and will be registered in
        // the index with this DocumentHandle. The block is guaranteed to be
        // available until the first call to DocHandle::Expire(). Returns a
        // pointer to the blob.
        void* AllocateVariableSizeBlob(VariableSizeBlobId id, size_t byteSize);

        // Returns a pointer to the variable-sized blob associated with a
        // given VariableSizeBlobId. The VariableSizeBlobId must have
        // been previouslyassigned by a call to
        //    IDocumentDataSchema::RegisterVariableSizeBlob().
        //
        // Returns nullptr if the blob data has not been previously allocated.
        void* GetVariableSizeBlob(VariableSizeBlobId id) const;

        // Returns a pointer to the fixed-sized blob associated with a
        // given FixedSizeBlobId. The FIxedSizeBlobId must have been previously
        // assigned by a call to
        //    IDocumentDataSchema::RegisterFixedSizeBlob().
        void* GetFixedSizeBlob(FixedSizeBlobId id) const;

        // Sets or clears a fact associated with the document. The FactHandle
        // must have been previously registered in the IFactSet.
        void AssertFact(FactHandle fact, bool value);

        // Adds a posting to the index, asserting that a Term is associated
        // with this document.
        void AddPosting(Term const & term);

        // Removes this document from the index. Queries initiated after
        // Expire() returns will not see this document. Queries already in
        // progress at the time Expire() is called may be able to see the
        // document.
        // Expires a document and removes it from serving. The document will no
        // longer be served from the queries, however it may still be physically
        // present in the index until it is recycled.
        // Always legal to call. Can't call multiple times on the same instance.
        //
        // Implementation:
        // bool isSliceExpired = m_slice->ExpireDocument(m_index);
        // if (isSliceExpired) m_slice->GetShard().RecycleSlice(m_slice);
        void Expire();

        // Returns the document's unique identifier that was assigned to it
        // at ingestion.
        DocId GetDocId() const;

        // This method exists so that IngestAndQuery REPL can display bits for
        // various rows. Not sure it is needed in the long run.
        bool GetBit(RowId row) const;

        // TODO: Methods for JIT trees.

    protected:
        // Constructor declared private to prevent public creation of the
        // instances of this class.
        DocumentHandle(Slice* slice, DocIndex index);

        // Slice where the document resides.
        Slice* m_slice;

        // Column in the slice where the document resides.
        DocIndex m_index;
    };
}
