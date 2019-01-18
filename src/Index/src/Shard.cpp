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
#include "BitFunnel/IFileManager.h"
#include "BitFunnel/Index/IRecycler.h"
#include "BitFunnel/Index/ISliceBufferAllocator.h"
#include "BitFunnel/Index/ITermTable.h"
#include "BitFunnel/Index/Row.h"
#include "BitFunnel/Index/RowIdSequence.h"
#include "BitFunnel/Index/Token.h"
#include "BitFunnel/Utilities/StreamUtilities.h"
#include "IRecyclable.h"
#include "LoggerInterfaces/Check.h"
#include "LoggerInterfaces/Logging.h"
#include "Recycler.h"
#include "Rounding.h"
#include "Shard.h"


namespace BitFunnel
{
    // Extracts a RowId used to mark documents as active/soft-deleted.
    static RowId RowIdForActiveDocument(ITermTable const & termTable)
    {
        RowIdSequence rows(ITermTable::GetDocumentActiveTerm(), termTable);

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


    Shard::Shard(ShardId id,
                 IRecycler& recycler,
                 ITokenManager& tokenManager,
                 ITermTable const & termTable,
                 IDocumentDataSchema const & docDataSchema,
                 ISliceBufferAllocator& sliceBufferAllocator,
                 size_t sliceBufferSize)
        : m_shardId(id),
          m_recycler(recycler),
          m_tokenManager(tokenManager),
          m_termTable(termTable),
          m_sliceBufferAllocator(sliceBufferAllocator),
          m_documentActiveRowId(RowIdForActiveDocument(termTable)),
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


    Shard::~Shard() {
        delete static_cast<std::vector<void*>*>(m_sliceBuffers);
    }


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


    void* Shard::LoadSliceBuffer(std::istream& input)
    {
        const size_t bufferSizePersisted = StreamUtilities::ReadField<size_t>(input);
        if (bufferSizePersisted != m_sliceBufferSize)
        {
            throw std::runtime_error("Data in the stream is not compatible with the current schema.");
        }

        // TODO: verify compatibility of DocTableDescriptor, RowTableDescriptor with the stream's data.

        void* buffer = m_sliceBufferAllocator.Allocate(m_sliceBufferSize);

        try
        {
            StreamUtilities::ReadBytes(input, buffer, m_sliceBufferSize);
        }
        catch (std::exception e)
        {
//            LogB(Logging::Error, "LoadSliceBuffer", "Error reading slice buffer data from stream");
            m_sliceBufferAllocator.Release(buffer);
            throw e;
        }

        return buffer;
    }


    // TODO: Should this really be in Shard? Seems it's only here because
    // m_sliceBufferSize is here.
    void Shard::WriteSliceBuffer(void* buffer, std::ostream& output)
    {
        // Write out the size of the slice buffer for compatibility check.
        StreamUtilities::WriteField<size_t>(output, m_sliceBufferSize);
        StreamUtilities::WriteBytes(output, reinterpret_cast<char*>(buffer), m_sliceBufferSize);

        // TODO: persist DocTableDescriptor, RowTableDescriptor compatibility information.
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
                                                            m_tokenManager));

        m_recycler.ScheduleRecyling(recyclableSliceList);
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
                Row::DocumentsInRank0Row(1, termTable.GetMaxRankUsed());
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


    ShardId Shard::GetId() const
    {
        return m_shardId;
    }


    DocIndex Shard::GetSliceCapacity() const
    {
        return  m_sliceCapacity;
    }


    size_t Shard::GetSliceBufferSize() const
    {
        return m_sliceBufferSize;
    }


    RowId Shard::GetDocumentActiveRowId() const
    {
        return m_documentActiveRowId;
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
        const Rank maxRank = termTable.GetMaxRankUsed();

        size_t currentOffset = 0;

        // A pointer to a Slice object is placed at the beginning of the slice buffer.
        currentOffset += sizeof(Slice*);

        //
        // DocTable
        //
        currentOffset = RoundUp(currentOffset, DocTableDescriptor::c_docTableByteAlignment);
        if (shard != nullptr)
        {
            // The cast of currentOffset is to avoid an implicit sign conversion
            // warning. It's possible that we should just make currentOffset
            // ptrdiff_t.
            shard->m_docTable.reset(new DocTableDescriptor(sliceCapacity,
                                                           docDataSchema,
                                                           static_cast<ptrdiff_t>(currentOffset)));
        }
        currentOffset += DocTableDescriptor::GetBufferSize(sliceCapacity, docDataSchema);

        //
        // RowTables
        //
        for (Rank rank = 0; rank <= c_maxRankValue; ++rank)
        {
            currentOffset = RoundUp(currentOffset, RowTableDescriptor::c_rowTableByteAlignment);

            const RowIndex rowCount = termTable.GetTotalRowCount(rank);

            if (shard != nullptr)
            {
                shard->m_rowTables.emplace_back(
                    sliceCapacity, rowCount, rank, maxRank, currentOffset);
            }

            currentOffset += RowTableDescriptor::GetBufferSize(
                sliceCapacity, rowCount, rank, maxRank);
        }

        const size_t sliceBufferSize = static_cast<size_t>(currentOffset);

        return sliceBufferSize;
    }


    void Shard::RecycleSlice(Slice& slice)
    {
        std::vector<void*>* oldSlices = nullptr;

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

            oldSlices = m_sliceBuffers.load();
            m_sliceBuffers = newSlices;

            if (m_activeSlice == &slice)
            {
                // If all of the above validations are true, then this was the
                // last Slice in the Shard.
                m_activeSlice = nullptr;
            }
        }

        // Scheduling the Slice and the old list of slice buffers can be
        // done outside of the lock.
        std::unique_ptr<IRecyclable>
            recyclableSliceList(new DeferredSliceListDelete(&slice,
                                                            oldSlices,
                                                            m_tokenManager));

        m_recycler.ScheduleRecyling(recyclableSliceList);
    }


    void Shard::ReleaseSliceBuffer(void* sliceBuffer)
    {
        m_sliceBufferAllocator.Release(sliceBuffer);
    }


    void Shard::AddPosting(Term const & term,
                           DocIndex index,
                           void* sliceBuffer)
    {
        // std::cout << "AddPosting shard:docIndex "
        //           << m_shardId << ":" << index << std::endl;

        if (m_docFrequencyTableBuilder.get() != nullptr)
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


    void Shard::AssertFact(FactHandle fact, bool value, DocIndex index, void* sliceBuffer)
    {
        Term term(fact, 0u, 1u);
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


    void Shard::TemporaryRecordDocument()
    {
        if (m_docFrequencyTableBuilder.get() != nullptr)
        {
            m_docFrequencyTableBuilder->OnDocumentEnter();
        }
    }


    void Shard::TemporaryWriteDocumentFrequencyTable(std::ostream& out,
                                                     ITermToText const * termToText) const
    {
        // TODO: 0.0 is the truncation frequency, which shouldn't be fixed at 0.
        if (m_docFrequencyTableBuilder.get() != nullptr)
        {
            m_docFrequencyTableBuilder->WriteFrequencies(out, 0.0, termToText);
        }
    }


    void Shard::TemporaryWriteCumulativeTermCounts(std::ostream& out) const
    {
        if (m_docFrequencyTableBuilder.get() != nullptr)
        {
            m_docFrequencyTableBuilder->WriteCumulativeTermCounts(out);
        }
    }


    // Reload a shard's saved slices, completely replacing whatever slices are in the shard
    // The last loaded slice will be the active slice
    void Shard::TemporaryReadAllSlices(IFileManager& fileManager, size_t nbrSlices)
    {
        auto token = m_tokenManager.RequestToken();

        std::vector<void*>* const newSlices = new std::vector<void*>();
        for (size_t i = 0; i < nbrSlices; ++i)
        {
            auto sliceFile = fileManager.IndexSlice(m_shardId, i);
            auto in = sliceFile.OpenForRead();
            Slice* newSlice = new Slice(*this, *in);
            newSlices->push_back(newSlice->GetSliceBuffer());
            m_activeSlice = newSlice;
        }

        std::vector<void*>* oldSlices = m_sliceBuffers;
        m_sliceBuffers = newSlices;

        // TODO: think if this can be done outside of the lock.
        std::unique_ptr<IRecyclable>
            recyclableSliceList(new DeferredSliceListDelete(nullptr,
                oldSlices,
                m_tokenManager));

        m_recycler.ScheduleRecyling(recyclableSliceList);
    }


    void Shard::TemporaryWriteAllSlices(IFileManager& fileManager) const
    {
        auto token = m_tokenManager.RequestToken();
        auto sliceBuffers = GetSliceBuffers();
        for (size_t i = 0; i < sliceBuffers.size(); ++i)
        {
            Slice* s = Slice::GetSliceFromBuffer(sliceBuffers[i],
                                                 GetSlicePtrOffset());

            auto out = fileManager.IndexSlice(m_shardId, i).OpenForWrite();
            s->Write(*out);
        }
    }


    //*************************************************************************
    //
    // DocumentHandle iterator.
    //
    //*************************************************************************
    Shard::ConstIterator::ConstIterator(Shard const & shard)
      : m_token(shard.m_tokenManager.RequestToken()),
        m_sliceBuffers(shard.m_sliceBuffers),
        m_sliceCapacity(shard.m_sliceCapacity),
        m_sliceIndex(0),
        m_slice(nullptr),
        m_sliceOffset(0)
    {
        // Advance to first active document, if not already at one.
        EnsureActive();
    }


    void Shard::ConstIterator::EnsureActive()
    {
        for (; !AtEnd(); ++m_sliceIndex)
        {
            m_slice = Slice::GetSliceFromBuffer(
                (*m_sliceBuffers)[m_sliceIndex],
                GetSlicePtrOffset());

            for (; m_sliceOffset < m_sliceCapacity; ++m_sliceOffset)
            {
                DocumentHandleInternal handle(m_slice, m_sliceOffset);
                if (handle.IsActive())
                {
                    return;
                }
            }
        }
    }


    bool Shard::ConstIterator::AtEnd() const
    {
        return m_sliceIndex >= m_sliceBuffers->size();
    }


    Shard::const_iterator& Shard::ConstIterator::operator++()
    {
        if (AtEnd())
        {
            RecoverableError
                error("Shard::ConstIterator::operator++: iterator beyond end.");
            throw error;
        }

        ++m_sliceOffset;
        EnsureActive();
        return *this;
    }


    DocumentHandle Shard::ConstIterator::operator*() const
    {
        if (AtEnd())
        {
            RecoverableError
                error("Shard::ConstIterator::operator*: iterator beyond end.");
            throw error;
        }

        return DocumentHandleInternal(m_slice, m_sliceOffset);
    }


    std::unique_ptr<IShard::const_iterator> Shard::GetIterator()
    {
        std::unique_ptr<IShard::const_iterator> it(new ConstIterator(*this));
        return it;
    }


    //*************************************************************************
    //
    // GetDensities
    //
    //*************************************************************************
    std::vector<double> Shard::GetDensities(Rank rank) const
    {
        // Hold a token to ensure that m_sliceBuffers won't be recycled.
        auto token = m_tokenManager.RequestToken();

        // m_sliceBuffers can change at any time, but we can safely grab the
        // pointer because
        //   1. m_sliceBuffers is std::atomic
        //   2. token guarantees that m_sliceBuffers pointer cannot be recycled.
        std::vector<void*> const & buffers = *m_sliceBuffers;

        RowTableDescriptor const & rowTable = m_rowTables[rank];
        RowTableDescriptor const & rowTable0 = m_rowTables[0];

        RowIndex active = (*RowIdSequence(ITermTable::GetDocumentActiveTerm(),
                                          m_termTable).begin()).GetIndex();

        std::vector<double> densities;
        for (RowIndex row = 0; row < rowTable.GetRowCount(); ++row)
        {
            size_t activeBitCount = 0;
            size_t setBitCount = 0;
            for (auto buffer : buffers)
            {
                for (DocIndex doc = 0; doc < GetSliceCapacity(); ++doc)
                {
                    // Only count bits for active documents.
                    if (rowTable0.GetBit(buffer, active, doc) != 0)
                    {
                        // This document is active in the index.
                        // Go ahead and include its bits in the density
                        // calculation.
                        ++activeBitCount;
                        if (rowTable.GetBit(buffer, row, doc) != 0)
                        {
                            ++setBitCount;
                        }
                    }
                }
            }
            double density =
                (activeBitCount == 0) ?
                0.0 :
                static_cast<double>(setBitCount) / activeBitCount;

            densities.push_back(density);
        }

        return densities;
    }


    // static
    ptrdiff_t Shard::GetSlicePtrOffset()
    {
        // The slice pointer is at the beginning of the buffer.
        return static_cast<ptrdiff_t>(0ull);
    }
}
