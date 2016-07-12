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


// TODO: port this test.

#include "stdafx.h"

#include <set>

#include "BitFunnel/Factories.h"
#include "BitFunnel/IThreadManager.h"
#include "BitFunnel/Row.h"
#include "DocumentDataSchema.h"
#include "EmptyTermTable.h"
#include "ExpectException.h"
#include "IngestionIndex.h"
#include "IndexWrapper.h"
#include "Rounding.h"
#include "Slice.h"
#include "Shard.h"
#include "SuiteCpp/UnitTest.h"
#include "TrackingSliceBufferAllocator.h"

namespace BitFunnel
{
    namespace ShardUnitTest
    {
        const size_t c_blockAllocatorBlockCount = 10;

        void TestSliceBuffers(Shard const & shard, std::vector<Slice*> const & allocatedSlices)
        {
            std::vector<void*> const & sliceBuffers = shard.GetSliceBuffers();
            TestEqual(sliceBuffers.size(), allocatedSlices.size());

            for (size_t i = 0; i < sliceBuffers.size(); ++i)
            {
                TestEqual(allocatedSlices[i]->GetSliceBuffer(), sliceBuffers[i]);

                // Slice buffer should contain a pointer to a Slice at the offset indicated by Shard.
                char* sliceBuffer = reinterpret_cast<char*>(sliceBuffers[i]);
                void** slicePtrInSliceBuffer = reinterpret_cast<void**>(sliceBuffer + shard.GetSlicePtrOffset());
                TestEqual(allocatedSlices[i], *slicePtrInSliceBuffer);
            }
        }


        TestCase(BasicTest)
        {
            static const DocIndex c_sliceCapacity = Row::DocumentsInRank0Row(1);

            static const std::vector<RowIndex> rowCounts = { 100, 0, 0, 200, 0, 0, 300 };
            std::shared_ptr<ITermTable const> termTable(new EmptyTermTable(rowCounts));

            DocumentDataSchema schema;

            IndexWrapper index(c_sliceCapacity, termTable, schema, c_blockAllocatorBlockCount);
            Shard& shard = index.GetShard();

            TestEqual(&shard.GetIndex(), &index.GetIndex());
            TestEqual(shard.GetSliceCapacity(), c_sliceCapacity);

            Slice* currentSlice = nullptr;
            std::vector<Slice*> allocatedSlices;
            TestSliceBuffers(shard, allocatedSlices);

            for (DocIndex i = 0; i < c_sliceCapacity * 3; ++i)
            {
                DocId docId = static_cast<DocId>(i) + 1234;

                const DocumentHandleInternal handle = shard.AllocateDocument();

                if ((i % c_sliceCapacity) == 0)
                {
                    if (currentSlice != nullptr)
                    {
                        // We must have advanced to another slice, so should have a new value
                        // of the Slice*.
                        TestNotEqual(handle.GetSlice(), currentSlice);
                    }

                    currentSlice = handle.GetSlice();
                    allocatedSlices.push_back(currentSlice);
                }

                currentSlice->GetDocTable().SetDocId(currentSlice->GetSliceBuffer(),
                                                     i % c_sliceCapacity,
                                                     docId);

                TestEqual(handle.GetDocId(), docId);
                TestEqual(handle.GetIndex(), i % c_sliceCapacity);
                TestEqual(handle.GetSlice(), currentSlice);

                TestSliceBuffers(shard, allocatedSlices);
            }
        }


        // A thread that simulates query thread activity. This thread simulates bits operations in
        // the RowTables across all ranks and DocTable operations in all DocIndex values in all
        // slice buffers in the Shard. Design intent is to simulate highly intense query thread
        // activity which would test the slice buffers with high frequency, while performing adding
        // or removing of slice buffers. Adding/removing os slice buffers has to be thread safe and
        // a query thread should never access the memory which has been deallocated.
        class QueryThread : public IThreadBase
        {
        public:
            QueryThread(Shard& shard, volatile bool& isExiting);

            virtual void EntryPoint() override;

        private:
            Shard& m_shard;
            volatile bool& m_isExiting;
        };


        QueryThread::QueryThread(Shard& shard, volatile bool& isExiting)
            : m_shard(shard),
              m_isExiting(isExiting)
        {
        }


        void QueryThread::EntryPoint()
        {
            for (;;)
            {
                if (m_isExiting)
                {
                    return;
                }

                ITermTable const & termTable(m_shard.GetTermTable());

                const Token token = m_shard.GetIndex().GetTokenManager().RequestToken();

                std::vector<void*> const & sliceBuffers = m_shard.GetSliceBuffers();
                for (const auto sliceBuffer : sliceBuffers)
                {
                    for (Rank r = 0; r <= c_maxRankValue; ++r)
                    {
                        RowTableDescriptor const & rowTable = m_shard.GetRowTable(r);
                        const unsigned quadwordCount = m_shard.GetSliceCapacity() / (1 << (6 + r));

                        const RowIndex rowCount = termTable.GetTotalRowCount(DDRTier, r);
                        for (RowIndex row = 0; row < rowCount; ++row)
                        {
                            unsigned __int64 *quadword = reinterpret_cast<unsigned __int64*>(
                                reinterpret_cast<char*>(sliceBuffer) + rowTable.GetRowOffset(row));

                            for (unsigned i = 0; i < quadwordCount; ++i)
                            {
                                // Don't need the value of the quadword, just need to simulate access to it.
                                *quadword;
                                ++quadword;
                            }
                        }
                    }

                    // Simulate access to DocTable.
                    DocTableDescriptor const & docTable = m_shard.GetDocTable();
                    for (DocIndex i = 0; i < m_shard.GetSliceCapacity(); ++i)
                    {
                        docTable.GetDocId(sliceBuffer, i);
                    }
                }
            }
        }


        // Returns the buffer size required to host a Slice with given schema properties.
        size_t GetRequiredBufferSize(DocIndex capacity,
                                     IDocumentDataSchema const & docDataSchema,
                                     ITermTable const & termTable)
        {
            return Shard::InitializeDescriptors(nullptr, capacity, docDataSchema, termTable);
        }


        TestCase(AddRemoveSliceTest)
        {
            // A number of thread that simulate querying activity.
            static const unsigned c_threadCount = 10;

            // Timeout for query threads to exit after they have been signaled to.
            static const unsigned c_queryThreadsExitTimeoutMS = 10000;

            static const DocIndex c_sliceCapacity = Row::DocumentsInRank0Row(1);

            static const std::vector<RowIndex> rowCounts = { 100, 0, 0, 200, 0, 0, 300 };
            std::shared_ptr<ITermTable const> termTable(new EmptyTermTable(rowCounts));

            DocumentDataSchema schema;

            const size_t sliceBufferSize = GetRequiredBufferSize(c_sliceCapacity, schema, *termTable);
            std::unique_ptr<ISliceBufferAllocator> sliceBufferAllocator(
                new TrackingSliceBufferAllocator(sliceBufferSize));
            TrackingSliceBufferAllocator& trackingAllocator
                = dynamic_cast<TrackingSliceBufferAllocator&>(*sliceBufferAllocator);

            IndexWrapper index(c_sliceCapacity, termTable, schema, sliceBufferAllocator);
            Shard& shard = index.GetShard();

            // A signal for query threads to exit.
            volatile bool isExiting = false;

            // Create query threads.
            std::vector<IThreadBase*> queryThreads;
            for (unsigned i = 0; i < c_threadCount; ++i)
            {
                queryThreads.push_back(new QueryThread(shard, isExiting));
            }

            std::unique_ptr<IThreadManager> threadManager(Factories::CreateThreadManager(queryThreads));

            Slice* currentSlice = nullptr;
            std::vector<Slice*> allocatedSlices;
            TestSliceBuffers(shard, allocatedSlices);

            for (DocIndex i = 0; i < c_sliceCapacity * c_blockAllocatorBlockCount; ++i)
            {
                const DocId docId = static_cast<DocId>(i) + 1234;

                const DocumentHandleInternal handle = shard.AllocateDocument();

                if ((i % c_sliceCapacity) == 0)
                {
                    if (currentSlice != nullptr)
                    {
                        // We must have advanced to another slice, so should have a new value
                        // of the Slice*.
                        TestNotEqual(handle.GetSlice(), currentSlice);
                    }

                    currentSlice = handle.GetSlice();
                    allocatedSlices.push_back(currentSlice);
                    TestEqual(trackingAllocator.GetInUseBuffersCount(), allocatedSlices.size());
                }

                currentSlice->GetDocTable().SetDocId(currentSlice->GetSliceBuffer(),
                                                     i % c_sliceCapacity,
                                                     docId);

                currentSlice->CommitDocument(handle.GetIndex());

                TestEqual(handle.GetDocId(), docId);
                TestEqual(handle.GetIndex(), i % c_sliceCapacity);
                TestEqual(handle.GetSlice(), currentSlice);

                TestSliceBuffers(shard, allocatedSlices);
                TestEqual(shard.GetUsedCapacityInBytes(), allocatedSlices.size() * sliceBufferSize);
            }

            // Start removing slices one by one.
            while (!allocatedSlices.empty())
            {
                Slice* const slice = allocatedSlices.back();

                for (DocIndex i = 0; i < c_sliceCapacity; ++i)
                {
                    if (i == c_sliceCapacity - 1)
                    {
                        // Trying to recycle non-expired slice buffer - expect exception.
                        ExpectException([&]()
                        {
                            shard.RecycleSlice(*slice);
                        });
                    }

                    slice->ExpireDocument(i);
                }

                TestSliceBuffers(shard, allocatedSlices);

                shard.RecycleSlice(*slice);
                allocatedSlices.pop_back();

                TestSliceBuffers(shard, allocatedSlices);
                TestEqual(shard.GetUsedCapacityInBytes(), allocatedSlices.size() * sliceBufferSize);
            }

            isExiting = true;

            const bool hasQueryThreadsExited = threadManager->WaitForThreads(c_queryThreadsExitTimeoutMS);
            TestAssert(hasQueryThreadsExited);

            TestEqual(trackingAllocator.GetInUseBuffersCount(), 0);

            // Trying to recycle a Slice which is not known to Shard - expect exception.
            // First add at least one Slice.
            shard.AllocateDocument();

            Slice slice(shard);
            for (DocIndex i = 0; i < shard.GetSliceCapacity(); ++i)
            {
                DocIndex docIndex = 0;
                TestAssert(slice.TryAllocateDocument(docIndex));
                slice.CommitDocument(docIndex);

                const bool isExpired = slice.ExpireDocument(docIndex);
                TestAssert(isExpired == (i == shard.GetSliceCapacity() - 1));
            }

            ExpectException([&] ()
            {
                shard.RecycleSlice(slice);
            });
        }


        size_t GetBufferSize(DocIndex capacity,
                             std::vector<RowIndex> const & rowCounts,
                             IDocumentDataSchema const & schema)
        {
            EmptyTermTable termTable(rowCounts);
            return Shard::InitializeDescriptors(nullptr, capacity, schema, termTable);
        }


        TestCase(BufferSizeTest)
        {
            static const DocIndex c_capacityQuanta = 4096;
            static const size_t c_sizeOfSlicePtr = sizeof(void*);
            DocumentDataSchema schema;
            schema.RegisterVariableSizeBlob();
            schema.RegisterFixedSizeBlob(10);

            {
                // When there are no rows, buffer size is driven only by DocTable.
                const size_t expectedBufferSize = DocTableDescriptor::GetBufferSize(c_capacityQuanta * 1, schema) + c_sizeOfSlicePtr;
                TestEqual(GetBufferSize(c_capacityQuanta * 1, { 0, 0, 0, 0, 0, 0, 0 }, schema), expectedBufferSize);
            }

            {
                // 1 row at rank 0.
                const size_t expectedBufferSize = DocTableDescriptor::GetBufferSize(c_capacityQuanta * 1, schema) + 4096 / 8 + c_sizeOfSlicePtr;
                TestEqual(GetBufferSize(c_capacityQuanta * 1, { 1, 0, 0, 0, 0, 0, 0 }, schema), expectedBufferSize);
            }

            {
                // 10 row at rank 0.
                const size_t expectedBufferSize = DocTableDescriptor::GetBufferSize(c_capacityQuanta * 1, schema) + 4096 / 8 * 10 + c_sizeOfSlicePtr;
                TestEqual(GetBufferSize(c_capacityQuanta * 1, { 10, 0, 0, 0, 0, 0, 0 }, schema), expectedBufferSize);
            }

            {
                // 1 row at rank 0, 10 rows at rank 3.
                const size_t expectedBufferSize = DocTableDescriptor::GetBufferSize(c_capacityQuanta * 1, schema) + 4096 / 8 + 4096 / 8 / 8 * 10 + c_sizeOfSlicePtr;
                TestEqual(GetBufferSize(c_capacityQuanta * 1, { 1, 0, 0, 10, 0, 0, 0 }, schema), expectedBufferSize);
            }

            {
                // 20 rows at rank 6.
                const size_t expectedBufferSize = DocTableDescriptor::GetBufferSize(c_capacityQuanta * 2, schema) + 2 * 4096 / 8 / 64 * 20 + c_sizeOfSlicePtr;
                TestEqual(GetBufferSize(c_capacityQuanta * 2, { 0, 0, 0, 0, 0, 0, 20 }, schema), expectedBufferSize);
            }
        }


        TestCase(CapacityForBufferSizeTest)
        {
            static const DocIndex c_capacityQuanta = 4096;
            static const size_t c_sizeOfSlicePtr = sizeof(void*);
            DocumentDataSchema schema;
            schema.RegisterVariableSizeBlob();
            schema.RegisterFixedSizeBlob(10);

            // Size of the DocTable entry is 8 + 8 + 10, rounded up to power of 2 is 32.
            // Min buffer size for DocTable is 32 * 4096 = 131072.

            const std::vector<RowIndex> rowCounts = { 100, 0, 0, 200, 0, 0, 0 };
            // 100 rows in rank 0 and 200 rows at rank 3 for 4096 documents make it
            // 100 * 4096 / 8 + 200 / 8 * 4096 / 8 = 51200 + 12800 = 64000.

            EmptyTermTable termTable(rowCounts);

            const size_t progressiveBufferSize = 131072 + 64000;
            const size_t minSliceBufferSize = progressiveBufferSize + c_sizeOfSlicePtr;

            TestEqual(Shard::GetCapacityForByteSize(minSliceBufferSize, schema, termTable), c_capacityQuanta);

            // Doubling the progressiveBufferSize is not enough to have 2 * c_capacityQuanta
            // since it does not accomodate the space for Slice*
            TestEqual(Shard::GetCapacityForByteSize(progressiveBufferSize * 2, schema, termTable), c_capacityQuanta);
            TestEqual(Shard::GetCapacityForByteSize(progressiveBufferSize * 2 + c_sizeOfSlicePtr, schema, termTable), c_capacityQuanta * 2);

            TestEqual(Shard::GetCapacityForByteSize(progressiveBufferSize * 3, schema, termTable), c_capacityQuanta * 2);
            TestEqual(Shard::GetCapacityForByteSize(progressiveBufferSize * 3 + c_sizeOfSlicePtr, schema, termTable), c_capacityQuanta * 3);

            // Same rule applies to any multiple of c_capacityQuanta.
            TestEqual(Shard::GetCapacityForByteSize(progressiveBufferSize * 123, schema, termTable), c_capacityQuanta * 122);
            TestEqual(Shard::GetCapacityForByteSize(progressiveBufferSize * 123 + c_sizeOfSlicePtr, schema, termTable), c_capacityQuanta * 123);
        }
    }
}
