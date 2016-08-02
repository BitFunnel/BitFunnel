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

#include "BitFunnel/BitFunnelTypes.h"        // for DocIndex, DocId
#include "BitFunnel/Index/DocumentHandle.h"


namespace BitFunnel
{
    class Slice;

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
        DocumentHandleInternal(Slice* slice, DocIndex index, DocId id);

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
