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

#include "BitFunnel/Exceptions.h"
#include "DocumentHandleInternal.h"
#include "Shard.h"                      // TODO: Remove this temporary include.
#include "Slice.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // DocumentHandle
    //
    //*************************************************************************
    void* DocumentHandle::AllocateVariableSizeBlob(VariableSizeBlobId /*id*/, size_t /*byteSize*/)
    {
        throw NotImplemented();
    }


    void* DocumentHandle::GetVariableSizeBlob(VariableSizeBlobId /*id*/) const
    {
        throw NotImplemented();
    }


    void* DocumentHandle::GetFixedSizeBlob(FixedSizeBlobId /*id*/) const
    {
        throw NotImplemented();
    }


    void DocumentHandle::AssertFact(FactHandle /*fact*/, bool /*value*/)
    {
        throw NotImplemented();
    }


    void DocumentHandle::AddPosting(Term const & term)
    {
        m_slice->GetShard().TemporaryAddPosting(term, m_index);
    }


    void DocumentHandle::Expire()
    {
        throw NotImplemented();
    }


    DocId DocumentHandle::GetDocId() const
    {
        throw NotImplemented();
    }


    DocumentHandle::DocumentHandle(Slice* slice, DocIndex index)
        : m_slice(slice),
          m_index(index)
    {
    }


    //*************************************************************************
    //
    // DocumentHandleInternal
    //
    //*************************************************************************

    DocumentHandleInternal::DocumentHandleInternal()
        : DocumentHandle(nullptr, c_invalidDocIndex)
    {
    }


    DocumentHandleInternal::DocumentHandleInternal(Slice* slice, DocIndex index)
        : DocumentHandle(slice, index)
    {
    }


    DocumentHandleInternal::DocumentHandleInternal(DocumentHandle const & handle)
        : DocumentHandle(handle)
    {
    }


    Slice* DocumentHandleInternal::GetSlice() const
    {
        return m_slice;
    }


    DocIndex DocumentHandleInternal::GetIndex() const
    {
        return m_index;
    }


    void DocumentHandleInternal::Activate()
    {
        throw NotImplemented();
    }
}
