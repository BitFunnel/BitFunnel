#pragma once

#include "BitFunnel/Index/DocumentHandle.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // Internal representation of the DocumentHandle. Instances of this class 
    // will be stored in DocumentMap. Extends DocumentHandle by allowing access
    // to Slice and DocIndex. Design intent is that DocumentHandles are 
    // created only by the BitFunnel library and are used in the public API. 
    // Internally BitFunnel library yses DocumentHandleInternal which is a 
    // subclass of DocumentHandle which gains access to protected members
    // Slice* and DocIndex.
    //
    // DESIGN NOTE: Instances of DocumentHandleInternal are only created for a
    // thread holding a token that guarantees that the Slice* remains valid for
    // the lifetime of the token.
    //
    //*************************************************************************
    class DocumentHandleInternal : public DocumentHandle
    {
    public:
        // Default constructor is added in order to use this class as a value
        // type for DocumentMap which uses it for uninitialized slots in the
        // hashtable.
        DocumentHandleInternal();

        // Constructs a handle from slice and offset (index) in the slice. 
        DocumentHandleInternal(Slice* slice, DocIndex index);

        // Copy constructor to convert from DocumentHandle. Required by 
        // IIndex::Add which converts the output of IIndex::AllocateDocument
        // from DocumentHandle to DocumentHandleInternal.
        DocumentHandleInternal(DocumentHandle const &);

        // Returns the slice where the document resides.
        Slice* GetSlice() const;

        // Returns the column in the slice where the document resides.
        DocIndex GetIndex() const;

        // Make the document visible to matcher. Must be called after the 
        // document's content is fully ingested.
        void Activate();
    };
}
