#include "DocumentHandleInternal.h"


namespace BitFunnel
{
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
        throw std::runtime_error("Not implemented.");
    }
}
