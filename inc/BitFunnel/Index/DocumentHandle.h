#pragma once

#include "BitFunnel/BitFunnelTypes.h"       // DocId is a return type.
#include "BitFunnel/IDocumentDataSchema.h"  // VariableSizeBlobId and FixedSizeBlobId are parameters.
#include "BitFunnel/IFactSet.h"             // FactHandle is a parameter.

namespace BitFunnel
{
    class Slice;
    class Term;

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