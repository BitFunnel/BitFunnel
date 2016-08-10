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
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Row.h"
#include "BitFunnel/TermInfo.h"
#include "IRecyclable.h"
#include "IRecycler.h"
#include "ISliceBufferAllocator.h"
#include "LoggerInterfaces/Logging.h"
#include "Recycler.h"
#include "Shard.h"
#include "BitFunnel/Term.h"       // TODO: Remove this temporary include.


namespace BitFunnel
{
    // Extracts a RowId used to mark documents as active/soft-deleted.
    static RowId RowIdForDeletedDocument(ITermTable const & termTable)
    {
        TermInfo termInfo(ITermTable::GetSoftDeletedTerm(), termTable);

        LogAssertB(termInfo.MoveNext(), "Invalid row.");
        const RowId rowId = termInfo.Current();

        LogAssertB(rowId.GetRank() == 0,
                   "Soft deleted row must be rank 0.");

        LogAssertB(!termInfo.MoveNext(),
                   "Soft deleted row must correspond to a single row.");

        return rowId;
    }


    Shard::Shard(IIngestor& ingestor,
                 size_t id,
                 ITermTable const & termTable,
                 IDocumentDataSchema const & docDataSchema,
                 ISliceBufferAllocator& sliceBufferAllocator,
                 size_t sliceBufferSize)
        : m_ingestor(ingestor),
          m_id(id),
          m_termTable(termTable),
          m_sliceBufferAllocator(sliceBufferAllocator),
          m_softDeletedRowId(RowIdForDeletedDocument(termTable)),
          m_activeSlice(nullptr),
          m_sliceBuffers(new std::vector<void*>()),
          m_sliceCapacity(GetCapacityForByteSize(sliceBufferSize,
                                                 docDataSchema,
                                                 termTable)),
          m_sliceBufferSize(sliceBufferSize),
          // TODO: will need one global, not one per shard.
          m_docFrequencyTableBuilder(new DocumentFrequencyTableBuilder())
    {
        const size_t bufferSize =
            InitializeDescriptors(this,
                                  m_sliceCapacity,
                                  docDataSchema,
                                  termTable);

        LogAssertB(bufferSize <= sliceBufferSize,
                   "Shard sliceBufferSize too small.");
    }


    Shard::~Shard() {}


    DocumentHandleInternal Shard::AllocateDocument(DocId id)
    {
        std::lock_guard<std::mutex> lock(m_slicesLock);
        DocIndex index;
        if (m_activeSlice == nullptr || !m_activeSlice->TryAllocateDocument(index))
        {
            CreateNewActiveSlice();

            LogAssertB(m_activeSlice->TryAllocateDocument(index),
                       "Newly allocated slice has no space.");
        }

        return DocumentHandleInternal(m_activeSlice, index, id);
    }


    void* Shard::AllocateSliceBuffer()
    {
        return m_sliceBufferAllocator.Allocate(m_sliceBufferSize);
    }

    // Must be called with m_slicesLock held.
    void Shard::CreateNewActiveSlice()
    {
        Slice* newSlice = new Slice(*this);

        std::vector<void*>* oldSlices = m_sliceBuffers;
        std::vector<void*>* const newSlices = new std::vector<void*>(*m_sliceBuffers);
        newSlices->push_back(newSlice->GetSliceBuffer());

        m_sliceBuffers = newSlices;
        m_activeSlice = newSlice;

        // TODO: think if this can be done outside of the lock.
        std::unique_ptr<IRecyclable>
            recyclableSliceList(new DeferredSliceListDelete(nullptr,
                                                            oldSlices,
                                                            GetIndex().GetTokenManager()));

        GetIndex().GetRecycler().ScheduleRecyling(recyclableSliceList);
    }


    /* static */
    DocIndex Shard::GetCapacityForByteSize(size_t bufferSizeInBytes,
                                           IDocumentDataSchema const & schema,
                                           ITermTable const & termTable)
    {
        DocIndex capacity = 0;
        for (;;)
        {
            const DocIndex newSuggestedCapacity = capacity +
                Row::DocumentsInRank0Row(1);
            const size_t newBufferSize =
                InitializeDescriptors(nullptr,
                                      newSuggestedCapacity,
                                      schema,
                                      termTable);
            if (newBufferSize > bufferSizeInBytes)
            {
                break;
            }

            capacity = newSuggestedCapacity;
        }

        LogAssertB(capacity > 0, "Shard with 0 capacity.");

        return capacity;
    }


    DocTableDescriptor const & Shard::GetDocTable() const
    {
        return *m_docTable;
    }


    IIngestor& Shard::GetIndex() const
    {
        return m_ingestor;
    }


    ptrdiff_t Shard::GetRowOffset(RowId rowId) const
    {
        // LogAssertB(rowId.IsValid(), "GetRowOffset on invalid row.");

        return GetRowTable(rowId.GetRank()).GetRowOffset(rowId.GetIndex());
    }


    RowTableDescriptor const & Shard::GetRowTable(Rank rank) const
    {
        return m_rowTables.at(rank);
    }


    std::vector<void*> const & Shard::GetSliceBuffers() const
    {
        return *m_sliceBuffers;
    }


    DocIndex Shard::GetSliceCapacity() const
    {
        return  m_sliceCapacity;
    }


    ptrdiff_t Shard::GetSlicePtrOffset() const
    {
        // A pointer to a Slice is placed in the end of the slice buffer.
        return m_sliceBufferSize - sizeof(void*);
    }


    RowId Shard::GetSoftDeletedRowId() const
    {
        return m_softDeletedRowId;
    }


    ITermTable const & Shard::GetTermTable() const
    {
        return m_termTable;
    }


    size_t Shard::GetUsedCapacityInBytes() const
    {
        // TODO: does this really need to be locked?
        std::lock_guard<std::mutex> lock(m_slicesLock);
        return m_sliceBuffers.load()->size() * m_sliceBufferSize;
    }


    /* static */
    size_t Shard::InitializeDescriptors(Shard* shard,
                                        DocIndex sliceCapacity,
                                        IDocumentDataSchema const & docDataSchema,
                                        ITermTable const & termTable)
    {
        ptrdiff_t currentOffset = 0;

        // Start of the DocTable is at offset 0.
        if (shard != nullptr)
        {
            shard->m_docTable.reset(new DocTableDescriptor(sliceCapacity,
                                                           docDataSchema,
                                                           currentOffset));
        }

        currentOffset += DocTableDescriptor::GetBufferSize(sliceCapacity, docDataSchema);

        for (Rank r = 0; r <= c_maxRankValue; ++r)
        {
            // TODO: see if this alignment matters.
            // currentOffset = RoundUp(currentOffset, c_rowTableByteAlignment);

            const RowIndex rowCount = termTable.GetTotalRowCount(r);

            if (shard != nullptr)
            {
                shard->m_rowTables.emplace_back(sliceCapacity, rowCount, r, currentOffset);
            }

            currentOffset += RowTableDescriptor::GetBufferSize(sliceCapacity, rowCount, r);
        }

        // A pointer to a Slice is placed at the end of the slice buffer.
        currentOffset += sizeof(void*);

        const size_t sliceBufferSize = static_cast<size_t>(currentOffset);

        return sliceBufferSize;
    }


    void Shard::RecycleSlice(Slice& slice)
    {
        std::vector<void*>* oldSlices = nullptr;
        size_t newSliceCount;

        {
            std::lock_guard<std::mutex> lock(m_slicesLock);

            if (!slice.IsExpired())
            {
                throw RecoverableError("Slice being recycled has not been fully expired");
            }

            std::vector<void*>* const newSlices = new std::vector<void*>();
            newSlices->reserve(m_sliceBuffers.load()->size() - 1);

            for (const auto it : *m_sliceBuffers)
            {
                if (it != slice.GetSliceBuffer())
                {
                    newSlices->push_back(it);
                }
            }

            if (m_sliceBuffers.load()->size() != newSlices->size() + 1)
            {
                throw RecoverableError("Slice buffer to be removed is not found in the active slice buffers list");
            }

            newSliceCount = newSlices->size();

            oldSlices = m_sliceBuffers.load();
            m_sliceBuffers = newSlices;

            if (m_activeSlice == &slice)
            {
                // If all of the above validations are true, then this was the
                // last Slice in the Shard.
                m_activeSlice = nullptr;
            }
        }

        // Logging outside of the lock.
        LogB(Logging::Info,
             "Shard",
             "Recycle slice for shard %u. New slice count is %u.",
             m_id,
             newSliceCount);

        // Scheduling the Slice and the old list of slice buffers can be
        // done outside of the lock.
        std::unique_ptr<IRecyclable>
            recyclableSliceList(new DeferredSliceListDelete(&slice,
                                                            oldSlices,
                                                            GetIndex().GetTokenManager()));

        GetIndex().GetRecycler().ScheduleRecyling(recyclableSliceList);
    }


    void Shard::ReleaseSliceBuffer(void* sliceBuffer)
    {
        m_sliceBufferAllocator.Release(sliceBuffer);
    }


    void Shard::TemporaryAddPosting(Term const & term, DocIndex /*index*/)
    {
        {
            // TODO: Remove this lock once it is incorporated into the frequency
            // table class.
            std::lock_guard<std::mutex> lock(m_temporaryFrequencyTableMutex);
            // m_temporaryFrequencyTable[term]++;
            m_docFrequencyTableBuilder->OnTerm(term);
        }
    }


    void Shard::TemporaryRecordDocument()
    {
        m_docFrequencyTableBuilder->OnDocumentEnter();
    }


    void Shard::TemporaryWriteDocumentFrequencyTable(std::ostream& out) const
    {
        // out << "Term frequency table for shard " << m_id << ":" << std::endl;
        // TODO: 0.0 is the truncation frequency, which shouldn't be fixed at 0.
        m_docFrequencyTableBuilder->WriteFrequencies(out, 0.0);
    }


    void Shard::TemporaryWriteCumulativePostingCounts(std::ostream& out) const
    {
        m_docFrequencyTableBuilder->WriteCumulativePostingCounts(out);
    }
}
