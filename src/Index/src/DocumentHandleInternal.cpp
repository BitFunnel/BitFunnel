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
#include "BitFunnel/TermInfo.h"
#include "DocumentHandleInternal.h"
#include "DocTableDescriptor.h"
#include "LoggerInterfaces/Logging.h"
#include "Shard.h"                      // TODO: Remove this temporary include.
#include "Slice.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // DocumentHandle
    //
    //*************************************************************************
    void* DocumentHandle::AllocateVariableSizeBlob(VariableSizeBlobId id, size_t byteSize)
    {
        return m_slice->GetDocTable().
            AllocateVariableSizeBlob(m_slice->GetSliceBuffer(),
                                     m_index,
                                     id,
                                     byteSize);
    }


    void* DocumentHandle::GetVariableSizeBlob(VariableSizeBlobId id) const
    {
        return m_slice->GetDocTable().
            GetVariableSizeBlob(m_slice->GetSliceBuffer(),
                                m_index,
                                id);
    }


    void* DocumentHandle::GetFixedSizeBlob(FixedSizeBlobId id) const
    {
        return m_slice->GetDocTable().
            GetFixedSizeBlob(m_slice->GetSliceBuffer(),
                             m_index,
                             id);
    }


    void DocumentHandle::AssertFact(FactHandle fact, bool value)
    {
        ITermTable const & termTable = m_slice->GetShard().GetTermTable();

        TermInfo termInfo(fact, termTable);

        LogAssertB(termInfo.MoveNext(),"Invalid FactHandle.");
        const RowId rowIdForFact = termInfo.Current();

        LogAssertB(!termInfo.MoveNext(),
                   "Fact must correspond to a single row.");

        RowTableDescriptor const & rowTable =
            m_slice->GetRowTable(rowIdForFact.GetRank());

        if (value)
        {
            rowTable.SetBit(m_slice->GetSliceBuffer(),
                            rowIdForFact.GetIndex(),
                            m_index);
        }
        else
        {
            rowTable.ClearBit(m_slice->GetSliceBuffer(),
                              rowIdForFact.GetIndex(),
                              m_index);
        }
    }


    void DocumentHandle::AddPosting(Term const & term)
    {
        m_slice->GetShard().TemporaryAddPosting(term, m_index);

        ITermTable const & termTable = m_slice->GetShard().GetTermTable();
        TermInfo termInfo(term, termTable);
        while (termInfo.MoveNext())
        {
            const RowId row = termInfo.Current();
            m_slice->GetRowTable(row.GetRank()).
                SetBit(m_slice->GetSliceBuffer(),
                       row.GetIndex(),
                       m_index);
        }
    }


    void DocumentHandle::Expire()
    {
        const RowId softDeletedRow = m_slice->GetShard().GetSoftDeletedRowId();

        RowTableDescriptor const & rowTable = m_slice->GetRowTable(softDeletedRow.GetRank());
        rowTable.ClearBit(m_slice->GetSliceBuffer(), softDeletedRow.GetIndex(), m_index);

        const bool isSliceExpired = m_slice->ExpireDocument();
        if (isSliceExpired)
        {
            // All documents are expired in the Slice and the index is
            // abandoning its reference to this Slice. If this was the only
            // reference, then the Slice is scheduled for backup.
            Slice::DecrementRefCount(m_slice);
        }
    }


    DocId DocumentHandle::GetDocId() const
    {
        return m_slice->GetDocTable().GetDocId(m_slice->GetSliceBuffer(),
                                               m_index);
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
        const RowId softDeletedRowId =
            m_slice->GetShard().GetSoftDeletedRowId();
        RowTableDescriptor const & rowTable =
            m_slice->GetRowTable(softDeletedRowId.GetRank());
        rowTable.SetBit(m_slice->GetSliceBuffer(),
                        softDeletedRowId.GetIndex(),
                        m_index);
    }
}
