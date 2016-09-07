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

#include "BitFunnel/ITermTable2.h"
#include "BitFunnel/RowId.h"
#include "BitFunnel/RowIdSequence.h"
#include "DocumentFrequencyTableBuilder.h"
#include "DocTableDescriptor.h"
#include "ISliceOwner.h"
#include "LoggerInterfaces/Logging.h"
#include "RowTableDescriptor.h"
#include "Slice.h"


namespace BitFunnel
{
    // TODO: where should this function live?
    // Extracts a RowId used to mark documents as active/soft-deleted.
    static RowId RowIdForDeletedDocument(ITermTable2 const & termTable)
    {
        RowIdSequence rows(termTable.GetDocumentActiveTerm(), termTable);

        auto it = rows.begin();
        if (it == rows.end())
        {
            RecoverableError error("RowIdForDeletedDocument: expected at least one row.");
            throw error;
        }
        const RowId rowId = *it;

        if (rowId.GetRank() != 0)
        {
            RecoverableError error("RowIdForDeletedDocument: soft deleted row must be rank 0..");
            throw error;
        }

        ++it;
        if (it != rows.end())
        {
            RecoverableError error("RowIdForDeletedDocument: expected no more than one row.");
            throw error;

        }

        return rowId;
    }
     

    Slice::Slice(ISliceOwner& owner,
                 DocumentFrequencyTableBuilder* docFrequencyTableBuilder,
                 ITermTable2 const & termTable,
                 DocTableDescriptor& docTable,
                 std::vector<RowTableDescriptor>& rowTables,
                 size_t sliceBufferSize,
                 DocIndex sliceCapacity,
                 void* sliceBuffer)
        : m_owner(owner),
          m_docFrequencyTableBuilder(docFrequencyTableBuilder),
          m_termTable(termTable),
          m_documentActiveRowId(RowIdForDeletedDocument(termTable)),
          m_temporaryNextDocIndex(0U),
          m_capacity(sliceCapacity),
          m_refCount(1),
          m_buffer(sliceBuffer),
          m_unallocatedCount(sliceCapacity),
          m_commitPendingCount(0),
          m_expiredCount(0),
          m_docTable(docTable),
          m_rowTables(rowTables),
          m_sliceBufferSize(sliceBufferSize)
    {
        Initialize();

        // Perform start up initialization of the DocTable and RowTables after
        // the buffer has been allocated.
        GetDocTable().Initialize(m_buffer);
        for (Rank r = 0; r <= c_maxRankValue; ++r)
        {
            GetRowTable(r).Initialize(m_buffer, m_termTable);
        }
    }


    Slice::~Slice()
    {
        try
        {
            GetDocTable().Cleanup(m_buffer);
            GetOwner().ReleaseSliceBuffer(m_buffer);
        }
        catch (...)
        {
            LogB(Logging::Error, "Slice", "Exception caught in Slice::~Slice()","");
        }
    }


    ptrdiff_t Slice::GetSlicePtrOffset() const
    {
        // A pointer to a Slice is placed in the end of the slice buffer.
        return m_sliceBufferSize - sizeof(void*);
    }


    RowId Slice::GetDocumentActiveRowId() const
    {
        return m_documentActiveRowId;
    }


    ISliceOwner& Slice::GetOwner() const
    {
        return m_owner;
    }


    void Slice::AddPosting(Term const & term, DocIndex index)
    {
        void* sliceBuffer = GetSliceBuffer();

         if (m_docFrequencyTableBuilder != nullptr)
         {
             m_docFrequencyTableBuilder->OnTerm(term);
         }

        RowIdSequence rows(term, m_termTable);

        for (auto const row : rows)
        {
            m_rowTables[row.GetRank()].SetBit(sliceBuffer,
                                              row.GetIndex(),
                                              index);
        }
    }


    void Slice::AssertFact(FactHandle fact, bool value, DocIndex index)
    {
        void* sliceBuffer = GetSliceBuffer();

        Term term(fact, 0u, 0u, 1u);
        RowIdSequence rows(term, m_termTable);
        auto it = rows.begin();

        if (it == rows.end())
        {
            RecoverableError error("Shard::AssertFact: expected at least one row.");
            throw error;
        }

        const RowId row = *it;

        ++it;
        if (it != rows.end())
        {
            RecoverableError error("Shard::AssertFact: expected no more than one row.");
            throw error;

        }

        RowTableDescriptor const & rowTable =
            m_rowTables[row.GetRank()];

        if (value)
        {
            rowTable.SetBit(sliceBuffer,
                            row.GetIndex(),
                            index);
        }
        else
        {
            rowTable.ClearBit(sliceBuffer,
                              row.GetIndex(),
                              index);
        }
    }


    bool Slice::CommitDocument()
    {
        if (m_docFrequencyTableBuilder != nullptr)
        {
            m_docFrequencyTableBuilder->OnDocumentEnter();
        }

        std::lock_guard<std::mutex> lock(m_docIndexLock);
        LogAssertB(m_commitPendingCount > 0,
                   "CommitDocument with m_commitPendingCount == 0");

        --m_commitPendingCount;

        return (m_unallocatedCount + m_commitPendingCount) == 0;
    }


    /* static */
    void Slice::DecrementRefCount(Slice* slice)
    {
        const unsigned newRefCount = --(slice->m_refCount);
        if (newRefCount == 0)
        {
            slice->GetOwner().RecycleSlice(*slice);
        }
    }


    bool Slice::ExpireDocument()
    {
        std::lock_guard<std::mutex> lock(m_docIndexLock);

        // Cannot expire more than what was committed.
        const DocIndex committedCount =
            m_capacity - m_unallocatedCount - m_commitPendingCount;
        LogAssertB(m_expiredCount < committedCount,
                   "Slice expired more documents than committed.");

        m_expiredCount++;

        return m_expiredCount == m_capacity;
    }


    DocTableDescriptor const & Slice::GetDocTable() const
    {
        return m_docTable;
    }


    RowTableDescriptor const & Slice::GetRowTable(Rank rank) const
    {
        return m_rowTables.at(rank);
    }


    void* Slice::GetSliceBuffer() const
    {
        return m_buffer;
    }


    /* static */
    Slice* Slice::GetSliceFromBuffer(void* sliceBuffer, ptrdiff_t slicePtrOffset)
    {
        return Slice::GetSlicePointer(sliceBuffer, slicePtrOffset);
    }


    // We have GetSlicePointer, which is private, so that the constructor can
    // get a reference and modify the pointer.
    /* static */
    Slice*& Slice::GetSlicePointer(void* sliceBuffer, ptrdiff_t slicePtrOffset)
    {
        char* slicePtr = reinterpret_cast<char*>(sliceBuffer) + slicePtrOffset;
        return *reinterpret_cast<Slice**>(slicePtr);
    }


    /* static */
    void Slice::IncrementRefCount(Slice* slice)
    {
        ++(slice->m_refCount);
    }


    void Slice::Initialize()
    {
        // Place a pointer to a Slice in the last bytes of the SliceBuffer.
        Slice*& slicePtr = GetSlicePointer(m_buffer, GetSlicePtrOffset());
        slicePtr = this;
    }


    bool Slice::IsExpired() const
    {
        return m_expiredCount == m_capacity;
    }


    bool Slice::TryAllocateDocument(size_t& index)
    {
        std::lock_guard<std::mutex> lock(m_docIndexLock);

        if (m_unallocatedCount == 0)
        {
            return false;
        }

        index = m_capacity - m_unallocatedCount;
        m_unallocatedCount--;
        m_commitPendingCount++;

        return true;
    }
}
