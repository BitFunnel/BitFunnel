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

#include <iostream>  // TODO: remove.

#include <future>

#include "gtest/gtest.h"

#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/ITermTable.h"
#include "BitFunnel/Row.h"
#include "BitFunnel/TermInfo.h"
#include "DocumentDataSchema.h"
#include "Ingestor.h"
#include "IRecycler.h"
#include "ISliceBufferAllocator.h"
#include "Mocks/EmptyTermTable.h"
#include "Recycler.h"
#include "Shard.h"
#include "Slice.h"
#include "TrackingSliceBufferAllocator.h"


namespace BitFunnel
{
    namespace ShardUnitTest
    {
        const size_t c_blockAllocatorBlockCount = 10;


        size_t GetBufferSize(DocIndex capacity,
                             std::vector<RowIndex> const & rowCounts,
                             IDocumentDataSchema const & schema)
        {
            EmptyTermTable termTable(rowCounts);
            return Shard::InitializeDescriptors(nullptr, capacity, schema, termTable);
        }


        void TestSliceBuffers(Shard const & shard, std::vector<Slice*> const & allocatedSlices)
        {
            std::vector<void*> const & sliceBuffers = shard.GetSliceBuffers();
            EXPECT_EQ(sliceBuffers.size(), allocatedSlices.size());

            for (size_t i = 0; i < sliceBuffers.size(); ++i)
            {
                EXPECT_EQ(allocatedSlices[i]->GetSliceBuffer(), sliceBuffers[i]);

                // Slice buffer should contain a pointer to a Slice at the offset indicated by Shard.
                char* sliceBuffer = reinterpret_cast<char*>(sliceBuffers[i]);
                void** slicePtrInSliceBuffer = reinterpret_cast<void**>(sliceBuffer + shard.GetSlicePtrOffset());
                EXPECT_EQ(allocatedSlices[i], *slicePtrInSliceBuffer);
            }
        }


        TEST(BasicShardTest, Trivial)
        {
            DocumentDataSchema schema;

            std::unique_ptr<IRecycler> recycler =
                std::unique_ptr<IRecycler>(new Recycler());
            auto background = std::async(std::launch::async, &IRecycler::Run, recycler.get());

            static const std::vector<RowIndex>
                rowCounts = { c_systemRowCount, 0, 0, 1, 0, 0, 1 };
            std::shared_ptr<ITermTable const>
                termTable(new EmptyTermTable(rowCounts));

            static const DocIndex c_sliceCapacity = Row::DocumentsInRank0Row(1);
            const size_t sliceBufferSize = GetBufferSize(c_sliceCapacity, rowCounts, schema);

            std::unique_ptr<TrackingSliceBufferAllocator> trackingAllocator(
                new TrackingSliceBufferAllocator(sliceBufferSize));

            const std::unique_ptr<IIngestor>
                ingestor(Factories::CreateIngestor(schema,
                                                   *recycler,
                                                   *termTable,
                                                   *trackingAllocator));

            Shard& shard = ingestor->GetShard(0);

            // EXPECT_EQ(&shard.GetIndex(), &index->GetIndex());
            EXPECT_EQ(shard.GetSliceCapacity(), c_sliceCapacity);

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
                        EXPECT_NE(handle.GetSlice(), currentSlice);
                    }

                    currentSlice = handle.GetSlice();
                    allocatedSlices.push_back(currentSlice);
                }

                currentSlice->GetDocTable().SetDocId(currentSlice->GetSliceBuffer(),
                                                     i % c_sliceCapacity,
                                                     docId);

                // TODO: fix this after docId is implemented.
                // EXPECT_EQ(handle.GetDocId(), docId);
                EXPECT_EQ(handle.GetIndex(), i % c_sliceCapacity);
                EXPECT_EQ(handle.GetSlice(), currentSlice);

                TestSliceBuffers(shard, allocatedSlices);
            }

            ingestor->Shutdown();
            recycler->Shutdown();
        }


        // Returns the buffer size required to host a Slice with given schema properties.
        size_t GetRequiredBufferSize(DocIndex capacity,
                                     IDocumentDataSchema const & docDataSchema,
                                     ITermTable const & termTable)
        {
            return Shard::InitializeDescriptors(nullptr, capacity, docDataSchema, termTable);
        }


        TEST(AddRemoveSliceTest, Trivial)
        {
            DocumentDataSchema schema;

            std::unique_ptr<IRecycler> recycler =
                std::unique_ptr<IRecycler>(new Recycler());
            auto background = std::async(std::launch::async, &IRecycler::Run, recycler.get());

            static const std::vector<RowIndex>
                rowCounts = { c_systemRowCount, 0, 0, 1, 0, 0, 1 };
            std::shared_ptr<ITermTable const>
                termTable(new EmptyTermTable(rowCounts));

            static const DocIndex c_sliceCapacity = Row::DocumentsInRank0Row(1);
            const size_t sliceBufferSize = GetBufferSize(c_sliceCapacity, rowCounts, schema);

            std::unique_ptr<TrackingSliceBufferAllocator> trackingAllocator(
                new TrackingSliceBufferAllocator(sliceBufferSize));

            const std::unique_ptr<IIngestor>
                ingestor(Factories::CreateIngestor(schema,
                                                   *recycler,
                                                   *termTable,
                                                   *trackingAllocator));

            Shard& shard = ingestor->GetShard(0);

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
                        EXPECT_NE(handle.GetSlice(), currentSlice);
                    }

                    currentSlice = handle.GetSlice();
                    allocatedSlices.push_back(currentSlice);
                    EXPECT_EQ(trackingAllocator->GetInUseBuffersCount(), allocatedSlices.size());
                }

                currentSlice->GetDocTable().SetDocId(currentSlice->GetSliceBuffer(),
                                                     i % c_sliceCapacity,
                                                     docId);

                currentSlice->CommitDocument();

                // TODO: restore after DocId is implemented.
                // EXPECT_EQ(handle.GetDocId(), docId);
                EXPECT_EQ(handle.GetIndex(), i % c_sliceCapacity);
                EXPECT_EQ(handle.GetSlice(), currentSlice);

                TestSliceBuffers(shard, allocatedSlices);
                EXPECT_EQ(shard.GetUsedCapacityInBytes(), allocatedSlices.size() * sliceBufferSize);
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
                        EXPECT_ANY_THROW(shard.RecycleSlice(*slice));
                    }

                    slice->ExpireDocument();
                }

                TestSliceBuffers(shard, allocatedSlices);

                shard.RecycleSlice(*slice);
                allocatedSlices.pop_back();

                TestSliceBuffers(shard, allocatedSlices);
                EXPECT_EQ(shard.GetUsedCapacityInBytes(), allocatedSlices.size() * sliceBufferSize);
            }

            // Totally arbitrary time to allow recycler thread to recycle in time.
            static const auto c_sleepTime = std::chrono::milliseconds(2);
            std::this_thread::sleep_for(c_sleepTime);
            EXPECT_EQ(trackingAllocator->GetInUseBuffersCount(), 0u);

            // Trying to recycle a Slice which is not known to Shard - expect exception.
            // First add at least one Slice.
            shard.AllocateDocument();

            Slice slice(shard);
            for (DocIndex i = 0; i < shard.GetSliceCapacity(); ++i)
            {
                DocIndex docIndex = 0;
                EXPECT_TRUE(slice.TryAllocateDocument(docIndex));
                slice.CommitDocument();

                const bool isExpired = slice.ExpireDocument();
                EXPECT_TRUE(isExpired == (i == shard.GetSliceCapacity() - 1));
            }

            EXPECT_ANY_THROW(shard.RecycleSlice(slice));

            ingestor->Shutdown();
            recycler->Shutdown();
        }


    //     TEST(BufferSizeTest, Trivial)
    //     {
    //         static const DocIndex c_capacityQuanta = 4096;
    //         static const size_t c_sizeOfSlicePtr = sizeof(void*);
    //         DocumentDataSchema schema;
    //         schema.RegisterVariableSizeBlob();
    //         schema.RegisterFixedSizeBlob(10);

    //         {
    //             // When there are no rows, buffer size is driven only by DocTable.
    //             const size_t expectedBufferSize = DocTableDescriptor::GetBufferSize(c_capacityQuanta * 1, schema) + c_sizeOfSlicePtr;
    //             EXPECT_EQ(GetBufferSize(c_capacityQuanta * 1, { 0, 0, 0, 0, 0, 0, 0 }, schema), expectedBufferSize);
    //         }

    //         {
    //             // 1 row at rank 0.
    //             const size_t expectedBufferSize = DocTableDescriptor::GetBufferSize(c_capacityQuanta * 1, schema) + 4096 / 8 + c_sizeOfSlicePtr;
    //             EXPECT_EQ(GetBufferSize(c_capacityQuanta * 1, { 1, 0, 0, 0, 0, 0, 0 }, schema), expectedBufferSize);
    //         }

    //         {
    //             // 10 row at rank 0.
    //             const size_t expectedBufferSize = DocTableDescriptor::GetBufferSize(c_capacityQuanta * 1, schema) + 4096 / 8 * 10 + c_sizeOfSlicePtr;
    //             EXPECT_EQ(GetBufferSize(c_capacityQuanta * 1, { 10, 0, 0, 0, 0, 0, 0 }, schema), expectedBufferSize);
    //         }

    //         {
    //             // 1 row at rank 0, 10 rows at rank 3.
    //             const size_t expectedBufferSize = DocTableDescriptor::GetBufferSize(c_capacityQuanta * 1, schema) + 4096 / 8 + 4096 / 8 / 8 * 10 + c_sizeOfSlicePtr;
    //             EXPECT_EQ(GetBufferSize(c_capacityQuanta * 1, { 1, 0, 0, 10, 0, 0, 0 }, schema), expectedBufferSize);
    //         }

    //         {
    //             // 20 rows at rank 6.
    //             const size_t expectedBufferSize = DocTableDescriptor::GetBufferSize(c_capacityQuanta * 2, schema) + 2 * 4096 / 8 / 64 * 20 + c_sizeOfSlicePtr;
    //             EXPECT_EQ(GetBufferSize(c_capacityQuanta * 2, { 0, 0, 0, 0, 0, 0, 20 }, schema), expectedBufferSize);
    //         }
    //     }


    //     TEST(CapacityForBufferSizeTest, Trivial)
    //     {
    //         static const DocIndex c_capacityQuanta = 4096;
    //         static const size_t c_sizeOfSlicePtr = sizeof(void*);
    //         DocumentDataSchema schema;
    //         schema.RegisterVariableSizeBlob();
    //         schema.RegisterFixedSizeBlob(10);

    //         // Size of the DocTable entry is 8 + 8 + 10, rounded up to power of 2 is 32.
    //         // Min buffer size for DocTable is 32 * 4096 = 131072.

    //         const std::vector<RowIndex> rowCounts = { 100, 0, 0, 200, 0, 0, 0 };
    //         // 100 rows in rank 0 and 200 rows at rank 3 for 4096 documents make it
    //         // 100 * 4096 / 8 + 200 / 8 * 4096 / 8 = 51200 + 12800 = 64000.

    //         EmptyTermTable termTable(rowCounts);

    //         const size_t progressiveBufferSize = 131072 + 64000;
    //         const size_t minSliceBufferSize = progressiveBufferSize + c_sizeOfSlicePtr;

    //         EXPECT_EQ(Shard::GetCapacityForByteSize(minSliceBufferSize, schema, termTable), c_capacityQuanta);

    //         // Doubling the progressiveBufferSize is not enough to have 2 * c_capacityQuanta
    //         // since it does not accomodate the space for Slice*
    //         EXPECT_EQ(Shard::GetCapacityForByteSize(progressiveBufferSize * 2, schema, termTable), c_capacityQuanta);
    //         EXPECT_EQ(Shard::GetCapacityForByteSize(progressiveBufferSize * 2 + c_sizeOfSlicePtr, schema, termTable), c_capacityQuanta * 2);

    //         EXPECT_EQ(Shard::GetCapacityForByteSize(progressiveBufferSize * 3, schema, termTable), c_capacityQuanta * 2);
    //         EXPECT_EQ(Shard::GetCapacityForByteSize(progressiveBufferSize * 3 + c_sizeOfSlicePtr, schema, termTable), c_capacityQuanta * 3);

    //         // Same rule applies to any multiple of c_capacityQuanta.
    //         EXPECT_EQ(Shard::GetCapacityForByteSize(progressiveBufferSize * 123, schema, termTable), c_capacityQuanta * 122);
    //         EXPECT_EQ(Shard::GetCapacityForByteSize(progressiveBufferSize * 123 + c_sizeOfSlicePtr, schema, termTable), c_capacityQuanta * 123);
    //     }
    // }
    }
}
