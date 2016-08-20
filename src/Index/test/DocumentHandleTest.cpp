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


#include <future>
#include <memory>

#include "gtest/gtest.h"

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/IRecycler.h"
#include "BitFunnel/Row.h"
#include "BitFunnel/Stream.h"
#include "BitFunnel/TermInfo.h"
#include "DocumentDataSchema.h"
#include "DocumentHandleInternal.h"
#include "IndexUtils.h"
#include "Ingestor.h"
#include "MockFileManager.h"
#include "MockTermTable.h"
#include "EmptyTermTable.h"
#include "Recycler.h"
#include "Slice.h"
#include "TrackingSliceBufferAllocator.h"

namespace BitFunnel
{
    // namespace DocumentHandleTest
    // {
    //     // Disabled this test because it is no longer legal to pass a bogus
    //     // Slice*. The constructor of DocumentHandle now registers the DocId
    //     // with the Slice.
    //     //
    //     //TEST(DocumentHandle, Basic)
    //     //{
    //     //    static Slice * const c_anySlice = reinterpret_cast<Slice*>(123);
    //     //    static const DocIndex c_anyDocIndex = 123;
    //     //    static const DocId c_anyDocId = 0;

    //     //    DocumentHandleInternal docHandle(c_anySlice, c_anyDocIndex, c_anyDocId);
    //     //    EXPECT_EQ(docHandle.GetSlice(), c_anySlice);
    //     //    EXPECT_EQ(docHandle.GetIndex(), c_anyDocIndex);
    //     //}


    //     struct FixedSizeBlob0
    //     {
    //         unsigned m_field1;
    //         float m_field2;
    //     };


    //     TEST(DocumentHandle, DocTableIntegration)
    //     {
    //         auto fileManager = CreateMockFileManager();

    //         DocumentDataSchema schema;
    //         const VariableSizeBlobId variableBlob =
    //             schema.RegisterVariableSizeBlob();
    //         const FixedSizeBlobId fixedSizeBlob =
    //             schema.RegisterFixedSizeBlob(sizeof(FixedSizeBlob0));

    //         std::unique_ptr<IRecycler> recycler =
    //             std::unique_ptr<IRecycler>(new Recycler());
    //         auto background = std::async(std::launch::async, &IRecycler::Run, recycler.get());

    //         static const std::vector<RowIndex>
    //             rowCounts = { 100, 0, 0, 200, 0, 0, 300, 0 };
    //         std::shared_ptr<ITermTable const>
    //             termTable(new EmptyTermTable(rowCounts));

    //         static const DocIndex c_sliceCapacity = Row::DocumentsInRank0Row(1);
    //         const size_t sliceBufferSize =
    //             GetBufferSize(c_sliceCapacity, schema, *termTable);

    //         auto shardDefinition = Factories::CreateShardDefinition();

    //         std::unique_ptr<TrackingSliceBufferAllocator> trackingAllocator(
    //             new TrackingSliceBufferAllocator(sliceBufferSize));

    //         const std::unique_ptr<IIngestor>
    //             ingestor(Factories::CreateIngestor(*fileManager,
    //                                                schema,
    //                                                *recycler,
    //                                                *termTable,
    //                                                *shardDefinition,
    //                                                *trackingAllocator));

    //         Shard& shard = ingestor->GetShard(0);

    //         Slice slice(shard);

    //         for (DocIndex docIndex = 0; docIndex < c_sliceCapacity; ++docIndex)
    //         {
    //             DocumentHandleInternal handle(&slice, docIndex, docIndex);

    //             // Simulate different size blobs.
    //             const size_t blobSize = 5 + docIndex / 100;

    //             void* blob = handle.AllocateVariableSizeBlob(variableBlob, blobSize);
    //             EXPECT_NE(blob, nullptr);
    //             memset(blob, 1, blobSize);

    //             void* blobTest = handle.GetVariableSizeBlob(variableBlob);
    //             EXPECT_EQ(blob, blobTest);

    //             uint8_t * blobPtr = reinterpret_cast<uint8_t*>(blob);
    //             for (size_t i = 0; i < blobSize; ++i)
    //             {
    //                 EXPECT_EQ(*blobPtr, 1u);
    //                 blobPtr++;
    //             }

    //             {
    //                 FixedSizeBlob0& fixedSizeBlobValue =
    //                     *static_cast<FixedSizeBlob0*>
    //                         (handle.GetFixedSizeBlob(fixedSizeBlob));
    //                 fixedSizeBlobValue.m_field1 = 222;
    //                 fixedSizeBlobValue.m_field2 = 333.0f;
    //             }

    //             {
    //                 FixedSizeBlob0 const & fixedSizeBlobValue
    //                     = *static_cast<FixedSizeBlob0*>
    //                         (handle.GetFixedSizeBlob(fixedSizeBlob));
    //                 EXPECT_EQ(fixedSizeBlobValue.m_field1, 222u);
    //                 EXPECT_EQ(fixedSizeBlobValue.m_field2, 333.0f);
    //             }
    //         }

    //         ingestor->Shutdown();
    //         recycler->Shutdown();
    //         background.wait();
    //     }

    //     // Helper method to get the RowId allocated for marking soft-deleted
    //     // documents.
    //     RowId RowIdForDeletedDocument(ITermTable const & termTable)
    //     {
    //         TermInfo termInfo(ITermTable::GetSoftDeletedTerm(), termTable);

    //         EXPECT_TRUE(termInfo.MoveNext());
    //         const RowId rowId = termInfo.Current();

    //         // Soft-deleted term must be in rank 0.
    //         EXPECT_EQ(rowId.GetRank(), 0u);

    //         // Soft-deleted term must correspond to a single row.
    //         EXPECT_FALSE(termInfo.MoveNext());

    //         return rowId;
    //     }


    //     Term CreateTestTerm(char const * termText)
    //     {
    //         return Term(Term::ComputeRawHash(termText), StreamId::Full, 0);
    //     }


    //     void AddTerm(MockTermTable& termTable, char const * termText)
    //     {
    //         const Term term(CreateTestTerm(termText));
    //         // TODO: 0 is arbitrary.
    //         termTable.AddTerm(term.GetRawHash(), 0, 1);
    //     }


    //     void AddTermAndVerify(DocumentHandleInternal handle, char const * termText)
    //     {
    //         const Term term(CreateTestTerm(termText));
    //         handle.AddPosting(term);

    //         Slice& slice = *handle.GetSlice();
    //         TermInfo termInfo(term, slice.GetShard().GetTermTable());
    //         ASSERT_FALSE(termInfo.IsEmpty());

    //         while (termInfo.MoveNext())
    //         {
    //             const RowId rowId = termInfo.Current();
    //             const uint64_t isBitSet = slice.
    //                 GetRowTable(rowId.GetRank()).GetBit(slice.GetSliceBuffer(),
    //                                                     rowId.GetIndex(),
    //                                                     handle.GetIndex());

    //             ASSERT_NE(isBitSet, 0u);
    //         }
    //     }


    //     void TestFact(DocumentHandleInternal handle, FactHandle fact)
    //     {
    //         Slice& slice = *handle.GetSlice();
    //         TermInfo termInfo(fact, slice.GetShard().GetTermTable());

    //         EXPECT_TRUE(termInfo.MoveNext());
    //         const RowId rowId = termInfo.Current();

    //         EXPECT_FALSE(termInfo.MoveNext());

    //         RowTableDescriptor const & rowTable =
    //             slice.GetRowTable(rowId.GetRank());
    //         bool isBitSet = rowTable.GetBit(slice.GetSliceBuffer(),
    //                                         rowId.GetIndex(),
    //                                         handle.GetIndex()) != 0;
    //         EXPECT_FALSE(isBitSet);

    //         handle.AssertFact(fact, true);

    //         isBitSet = rowTable.GetBit(slice.GetSliceBuffer(),
    //                                    rowId.GetIndex(),
    //                                    handle.GetIndex()) != 0;
    //         EXPECT_TRUE(isBitSet);

    //         handle.AssertFact(fact, false);
    //         isBitSet = rowTable.GetBit(slice.GetSliceBuffer(),
    //                                    rowId.GetIndex(),
    //                                    handle.GetIndex()) != 0;
    //         EXPECT_FALSE(isBitSet);
    //     }


    //     bool IsDocumentActive(DocumentHandleInternal const & handle,
    //                           RowId softDeletedDocumentRow)
    //     {
    //         const bool isBitSet = handle.GetSlice()->
    //             GetRowTable(softDeletedDocumentRow.
    //                         GetRank()).
    //             GetBit(handle.GetSlice()->GetSliceBuffer(),
    //                    softDeletedDocumentRow.GetIndex(),
    //                    handle.GetIndex()) > 0;
    //         return isBitSet;
    //     }


    //     TEST(DocumentHandle, RowTableIntegration)
    //     {
    //         auto fileManager = CreateMockFileManager();

    //         DocumentDataSchema schema;

    //         std::unique_ptr<IRecycler> recycler =
    //             std::unique_ptr<IRecycler>(new Recycler());
    //         auto background = std::async(std::launch::async, &IRecycler::Run, recycler.get());

    //         static const std::vector<RowIndex>
    //             // 4 rows for private terms, 1 row for a fact.
    //             rowCounts = { c_systemRowCount + 4 + 1, 0, 0, 0, 0, 0, 0 };
    //         std::shared_ptr<ITermTable const> termTable(new MockTermTable(0));
    //         MockTermTable& mockTermTable = const_cast<MockTermTable&>(
    //             dynamic_cast<MockTermTable const &>(*termTable));

    //         std::unique_ptr<IFactSet> facts(Factories::CreateFactSet());
    //         const FactHandle fact0 = facts->DefineFact("fact0", true);
    //         mockTermTable.AddRowsForFacts(*facts);

    //         AddTerm(mockTermTable, "this");
    //         AddTerm(mockTermTable, "is");
    //         AddTerm(mockTermTable, "a");
    //         AddTerm(mockTermTable, "test");

    //         static const DocIndex c_sliceCapacity = Row::DocumentsInRank0Row(1);
    //         const size_t sliceBufferSize = GetBufferSize(c_sliceCapacity, schema, *termTable);

    //         auto shardDefinition = Factories::CreateShardDefinition();

    //         std::unique_ptr<TrackingSliceBufferAllocator> trackingAllocator(
    //             new TrackingSliceBufferAllocator(sliceBufferSize));

    //         const std::unique_ptr<IIngestor>
    //             ingestor(Factories::CreateIngestor(*fileManager,
    //                                                schema,
    //                                                *recycler,
    //                                                *termTable,
    //                                                *shardDefinition,
    //                                                *trackingAllocator));

    //         Shard& shard = ingestor->GetShard(0);

    //         const RowId softDeletedDocumentRow = RowIdForDeletedDocument(*termTable);

    //         for (DocIndex i = 0; i < c_sliceCapacity; ++i)
    //         {
    //             DocumentHandleInternal handle = shard.AllocateDocument(i);

    //             // Document is not active untill fully ingested and activated.
    //             // Activation is done by the owning Index.
    //             EXPECT_FALSE(IsDocumentActive(handle, softDeletedDocumentRow));

    //             AddTermAndVerify(handle, "this");
    //             AddTermAndVerify(handle, "is");
    //             AddTermAndVerify(handle, "a");
    //             AddTermAndVerify(handle, "test");

    //             TestFact(handle, fact0);

    //             // Document is still not active.
    //             EXPECT_FALSE(IsDocumentActive(handle, softDeletedDocumentRow));

    //             handle.GetSlice()->CommitDocument();
    //             EXPECT_FALSE(IsDocumentActive(handle, softDeletedDocumentRow));

    //             // In order to verify that DocumentHandle::Expire clears the
    //             // soft-deleted bit, need to set this bit
    //             // manually. DocumentHandle itself does not set this bit - it is
    //             // done by the owning index, after all ingestion related logic
    //             // has completed - hence we need to manually set it here.
    //             handle.GetSlice()->
    //                 GetRowTable(softDeletedDocumentRow.GetRank()).
    //                 SetBit(handle.GetSlice()->GetSliceBuffer(),
    //                        softDeletedDocumentRow.GetIndex(),
    //                        handle.GetIndex());
    //             EXPECT_TRUE(IsDocumentActive(handle, softDeletedDocumentRow));

    //             handle.Expire();
    //             EXPECT_FALSE(IsDocumentActive(handle,
    //                                           softDeletedDocumentRow));
    //         }

    //         // We need to wait at least until recycling is scheduled to avoid
    //         // leaking our Slice. Sine we don't have a good way of checking if
    //         // recycling has been scheduled, we wait until recyling has
    //         // completed.
    //         while (trackingAllocator->GetInUseBuffersCount() != 0u) {}
    //         ingestor->Shutdown();
    //         recycler->Shutdown();
    //         background.wait();
    //     }

    //     // Fills up a Slice full of commited documents and returns a pointer to
    //     // this Slice.
    //     Slice* FillUpSlice(Shard& shard, DocIndex sliceCapacity)
    //     {
    //         Slice* slice = nullptr;
    //         for (DocIndex i = 0; i < sliceCapacity; ++i)
    //         {
    //             const DocumentHandleInternal handle = shard.AllocateDocument(i);

    //             if (slice == nullptr)
    //             {
    //                 slice = handle.GetSlice();
    //             }

    //             slice->CommitDocument();
    //         }

    //         return slice;
    //     }

    //     // Test to verify that expiring the last document in a Slice, schedules it for
    //     // recycling.
    //     TEST(DocumentHandle, ExpireTriggersRecycle)
    //     {
    //         auto fileManager = CreateMockFileManager();

    //         // Arbitrary amount of time to sleep in order to wait for Recycler.
    //         static const auto c_sleepTime = std::chrono::milliseconds(1);

    //         DocumentDataSchema schema;

    //         std::unique_ptr<IRecycler> recycler =
    //             std::unique_ptr<IRecycler>(new Recycler());
    //         auto background = std::async(std::launch::async, &IRecycler::Run, recycler.get());

    //         static const std::vector<RowIndex>
    //             rowCounts = { 100, 0, 0, 200, 0, 0, 300 };
    //         std::shared_ptr<ITermTable const>
    //             termTable(new MockTermTable(0));

    //         static const DocIndex c_sliceCapacity = Row::DocumentsInRank0Row(1);
    //         const size_t sliceBufferSize = GetBufferSize(c_sliceCapacity, schema, *termTable);

    //         auto shardDefinition = Factories::CreateShardDefinition();

    //         std::unique_ptr<TrackingSliceBufferAllocator> trackingAllocator(
    //             new TrackingSliceBufferAllocator(sliceBufferSize));

    //         const std::unique_ptr<IIngestor>
    //             ingestor(Factories::CreateIngestor(*fileManager,
    //                                                schema,
    //                                                *recycler,
    //                                                *termTable,
    //                                                *shardDefinition,
    //                                                *trackingAllocator));

    //         Shard& shard = ingestor->GetShard(0);

    //         std::this_thread::sleep_for(c_sleepTime);
    //         EXPECT_EQ(trackingAllocator->GetInUseBuffersCount(), 0u);

    //         {
    //             // Create a Slice and expire all documents. Verify it got recycled.
    //             Slice* currentSlice = FillUpSlice(shard, c_sliceCapacity);
    //             while (trackingAllocator->GetInUseBuffersCount() != 1u) {}

    //             // Expire all documents in the Slice. This should decrement ref count to 1.
    //             // The slice is still not recycled since there is one reference holder.
    //             for (DocIndex i = 0; i < c_sliceCapacity; ++i)
    //             {
    //                 DocumentHandleInternal handle(currentSlice, i, i);
    //                 handle.Expire();
    //             }

    //             // Verify that the Slice got recycled.
    //             while (trackingAllocator->GetInUseBuffersCount() != 0u) {}
    //         }

    //         {
    //             // This time, simulate that there is another reference holder of the Slice.
    //             Slice* currentSlice = FillUpSlice(shard, c_sliceCapacity);
    //             while (trackingAllocator->GetInUseBuffersCount() != 1u) {}

    //             // Simulate another reference holder of the slice, such as backup writer.
    //             Slice::IncrementRefCount(currentSlice);

    //             // The Slice should not be recycled since there are 2 reference holders.
    //             std::this_thread::sleep_for(c_sleepTime);

    //             // Expire all documents in the Slice. This should decrement ref count to 1.
    //             // The slice is still not recycled since there is one reference holder.
    //             for (DocIndex i = 0; i < c_sliceCapacity; ++i)
    //             {
    //                 DocumentHandleInternal handle(currentSlice, i, i);
    //                 handle.Expire();
    //             }

    //             // Verify that the Slice did not get recycled.
    //             std::this_thread::sleep_for(c_sleepTime);
    //             EXPECT_EQ(trackingAllocator->GetInUseBuffersCount(), 1u);

    //             // Decrement the last ref count, Slice should be scheduled for recycling.
    //             Slice::DecrementRefCount(currentSlice);
    //             while (trackingAllocator->GetInUseBuffersCount() != 0u) {}
    //         }

    //         ingestor->Shutdown();
    //         recycler->Shutdown();
    //         background.wait();
    //     }
    // }
}
