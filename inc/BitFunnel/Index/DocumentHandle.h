#pragma once

#include "BitFunnel/Index/IDocumentDataSchema.h"    // VariableSizeBlobId and FixedSizeBlobId are parameters.
#include "BitFunnel/Index/IFactSet.h"               // FactHandle is a parameter.

namespace BitFunnel
{
    class Slice;
    class Term;

    // BITFUNNELTYPES
    // A unique, system-wide document identifier.
    // For now we will make not assumptions about how DocIDs are assigned to
    // documents. The mapping may have significant and seemingly random gaps.
    // DESIGN NOTE: The concept of invalid documents (documents with invalid
    // document ID) was introduced to allow the ResultsProcessor to pass over
    // document positions used to pad the DocTable row length quanta. It also
    // provides a means to invalidate a document position after index
    // construction.
    typedef unsigned __int64 DocId;


    // BITFUNNELTYPES
    // A shard-independent document identifier which is local to a BitFunnel index.
    // DocIndex values in an index run from 0 to n where n-1 is the number of documents
    // in the index. The number of bits for DocIndex and ShardId together must not 
    // exceed 32.
    typedef unsigned __int32 DocIndex;


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
        // Allocates data for a blob with the given blob ID and a requested size.
        // The blob data is allocated on the heap and will be released when the 
        // document is recycled out of the index. Returns the pointer to the 
        // blob data. Blob ID must have been previously declared by a call to
        // IDocumentDataSchema::RegisterVariableSizeBlob, otherwise the function
        // throws.
        void* AllocateVariableSizeBlob(VariableSizeBlobId id, size_t size);

        // Returns the pointer to a variable size blob with the given ID in the
        // DocTable. Blob ID must have been previously declared by a call to
        // IDocumentDataSchema::RegisterVariableSizeBlob, otherwise the function
        // throws. Returns nullptr if the blob data has not been previously 
        // allocated.
        void* GetVariableSizeBlob(VariableSizeBlobId id) const;

        // Returns a pointer to the slot in the fixed size storage with the
        // given ID. A slot must have been previously registered during system
        // configuration with the call to
        // IDocumentDataSchema::RegisterFixedSizeBlob, otherwise the function
        // throws.
        void* GetFixedSizeBlob(FixedSizeBlobId id) const;

        // Sets or clears a fact about a document. Fact must have been previously
        // registered in the IFactSet, otherwise the function throws.
        void AssertFact(FactHandle fact, bool value);

        // Adds a posting to the index for the term.
        //
        // Implementation:
        //   TermTable const & termTable = m_slice->GetShard().GetTermTable();
        //   TermInfo(term, termTable);
        //   foreach (row : termInfo) m_slice->SetBit(row, m_index);
        void AddPosting(Term const & term);

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
