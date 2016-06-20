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
