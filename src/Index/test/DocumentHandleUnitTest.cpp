#include "stdafx.h"

#include <memory>

#include "BitFunnel/Factories.h"
#include "BitFunnel/Row.h"
#include "BitFunnel/TermInfo.h"
#include "DocumentDataSchema.h"
#include "DocumentHandleInternal.h"
#include "EmptyTermTable.h"
#include "IndexWrapper.h"
#include "MockTermTable.h"
#include "Slice.h"
#include "SuiteCpp/UnitTest.h"
#include "TrackingSliceBufferAllocator.h"

namespace BitFunnel
{
    namespace DocumentHandleUnitTest
    {
        static Slice * const c_anySlice = reinterpret_cast<Slice*>(123);
        static const DocIndex c_anyDocIndex = 123;

        TestCase(BasicTest)
        {
            DocumentHandleInternal docHandle(c_anySlice, c_anyDocIndex);
            TestEqual(docHandle.GetSlice(), c_anySlice);
            TestEqual(docHandle.GetIndex(), c_anyDocIndex);
        }


        struct FixedSizeBlob0
        {
            unsigned m_field1;
            float m_field2;
        };


        const size_t c_blockAllocatorBlockCount = 10;

        TestCase(DocTableIntegration)
        {
            static const DocIndex c_sliceCapacity = Row::DocumentsInRank0Row(1);

            static const std::vector<RowIndex> rowCounts = { 100, 0, 0, 200, 0, 0, 300 };
            std::shared_ptr<ITermTable const> termTable(new EmptyTermTable(rowCounts));

            DocumentDataSchema schema;
            const VariableSizeBlobId variableBlob = schema.RegisterVariableSizeBlob();
            const FixedSizeBlobId fixedSizeBlob = schema.RegisterFixedSizeBlob(sizeof(FixedSizeBlob0));

            IndexWrapper index(c_sliceCapacity, termTable, schema, c_blockAllocatorBlockCount);
            Shard& shard = index.GetShard();

            Slice slice(shard);
            
            for (DocIndex docIndex = 0; docIndex < c_sliceCapacity; ++docIndex)
            {
                DocumentHandleInternal handle(&slice, docIndex);

                // Simulate different size blobs.
                const size_t blobSize = 5 + docIndex / 100;

                void* blob = handle.AllocateVariableSizeBlob(variableBlob, blobSize);
                TestAssert(blob != nullptr);
                memset(blob, 1, blobSize);

                void* blobTest = handle.GetVariableSizeBlob(variableBlob);
                TestEqual(blob, blobTest);

                unsigned __int8 * blobPtr = reinterpret_cast<unsigned __int8*>(blob);
                for (size_t i = 0; i < blobSize; ++i)
                {
                    TestEqual(*blobPtr, 1u);
                    blobPtr++;
                }

                {
                    FixedSizeBlob0& fixedSizeBlobValue = *static_cast<FixedSizeBlob0*>(handle.GetFixedSizeBlob(fixedSizeBlob));
                    fixedSizeBlobValue.m_field1 = 222;
                    fixedSizeBlobValue.m_field2 = 333.0;
                }

                {
                    FixedSizeBlob0 const & fixedSizeBlobValue 
                        = *static_cast<FixedSizeBlob0*>(handle.GetFixedSizeBlob(fixedSizeBlob));
                    TestEqual(fixedSizeBlobValue.m_field1, 222u);
                    TestEqual(fixedSizeBlobValue.m_field2, 333.0);
                }
            }
        }


        // Helper method to get the RowId allocated for marking soft-deleted documents.
        RowId RowIdForDeletedDocument(ITermTable const & termTable)
        {
            TermInfo termInfo(ITermTable::GetSoftDeletedTerm(), termTable);

            TestAssert(termInfo.MoveNext());
            const RowId rowId = termInfo.Current();

            // Soft-deleted term must be in DDR tier and rank 0.
            TestAssert(rowId.GetRank() == 0);
            TestAssert(rowId.GetTier() == DDRTier);

            // Soft-deleted term must correspond to a single row.
            TestAssert(!termInfo.MoveNext());

            return rowId;
        }


        Term CreateTestTerm(char const * termText)
        {
            return Term(Term::ComputeRawHash(termText), Stream::Full, 0, DDRTier);
        }


        void AddTerm(MockTermTable& termTable, char const * termText)
        {
            const Term term(CreateTestTerm(termText));

            // MockTermTable allocates a random set of rows the first time a term
            // is queries. So to allocate rows for a term, it is sufficient to 
            // create a TermInfo object for it that will tigger query to the 
            // term table.
            TermInfo(term, termTable);
        }


        void AddTermAndVerify(DocumentHandleInternal handle, char const * termText)
        {
            const Term term(CreateTestTerm(termText));
            handle.AddPosting(term);

            Slice& slice = *handle.GetSlice();
            TermInfo termInfo(term, slice.GetShard().GetTermTable());
            TestAssert(!termInfo.IsEmpty());

            while (termInfo.MoveNext())
            {
                const RowId rowId = termInfo.Current();
                TestEqual(rowId.GetTier(), DDRTier);
                const char isBitSet = slice.GetRowTable(rowId.GetRank()).GetBit(slice.GetSliceBuffer(), 
                                                                                rowId.GetIndex(), 
                                                                                handle.GetIndex());

                TestAssert(isBitSet > 0);
            }
        }


        void TestFact(DocumentHandleInternal handle, FactHandle fact)
        {
            Slice& slice = *handle.GetSlice();
            TermInfo termInfo(fact, slice.GetShard().GetTermTable());

            TestAssert(termInfo.MoveNext());
            const RowId rowId = termInfo.Current();

            TestAssert(!termInfo.MoveNext());

            RowTableDescriptor const & rowTable = slice.GetRowTable(rowId.GetRank());
            bool isBitSet = rowTable.GetBit(slice.GetSliceBuffer(),
                                            rowId.GetIndex(),
                                            handle.GetIndex()) > 0;
            TestAssert(!isBitSet);

            handle.AssertFact(fact, true);

            isBitSet = rowTable.GetBit(slice.GetSliceBuffer(),
                                       rowId.GetIndex(),
                                       handle.GetIndex()) > 0;
            TestAssert(isBitSet);

            handle.AssertFact(fact, false);
            isBitSet = rowTable.GetBit(slice.GetSliceBuffer(),
                                       rowId.GetIndex(),
                                       handle.GetIndex()) > 0;
            TestAssert(!isBitSet);
        }


        bool IsDocumentActive(DocumentHandleInternal const & handle,
                              RowId softDeletedDocumentRow)
        {
            const bool isBitSet = handle.GetSlice()->GetRowTable(softDeletedDocumentRow.GetRank()).GetBit(handle.GetSlice()->GetSliceBuffer(),
                                                                                                          softDeletedDocumentRow.GetIndex(),
                                                                                                          handle.GetIndex()) > 0;
            return isBitSet;
        }


        TestCase(RowTableIntegration)
        {
            static const DocIndex c_sliceCapacity = Row::DocumentsInRank0Row(1);

            std::shared_ptr<ITermTable const> termTable(new MockTermTable(0));
            MockTermTable& mockTermTable = const_cast<MockTermTable&>(
                dynamic_cast<MockTermTable const &>(*termTable));
            AddTerm(mockTermTable, "this");
            AddTerm(mockTermTable, "is");
            AddTerm(mockTermTable, "a");
            AddTerm(mockTermTable, "test");

            std::unique_ptr<IFactSet> facts(Factories::CreateFactSet());
            const FactHandle fact0 = facts->DefineFact("fact0", true);
            mockTermTable.AddRowsForFacts(*facts);

            const RowId softDeletedDocumentRow = RowIdForDeletedDocument(*termTable);

            DocumentDataSchema schema;
            IndexWrapper index(c_sliceCapacity, termTable, schema, c_blockAllocatorBlockCount);
            Shard& shard = index.GetShard();

            for (DocIndex i = 0; i < c_sliceCapacity; ++i)
            {
                DocumentHandleInternal handle = shard.AllocateDocument();

                // Document is not active untill fully ingested and activated.
                // Activation is done by the owning Index.
                TestAssert(!IsDocumentActive(handle, softDeletedDocumentRow));

                AddTermAndVerify(handle, "this");
                AddTermAndVerify(handle, "is");
                AddTermAndVerify(handle, "a");
                AddTermAndVerify(handle, "test");

                TestFact(handle, fact0);

                // Document is still not active.
                TestAssert(!IsDocumentActive(handle, softDeletedDocumentRow));

                handle.GetSlice()->CommitDocument(handle.GetIndex());
                TestAssert(!IsDocumentActive(handle, softDeletedDocumentRow));

                // In order to verify that DocumentHandle::Expire clears the soft-deleted bit, need to set
                // this bit manually. DocumentHandle itself does not set this bit - it is done by the owning index, 
                // after all ingestion related logic has completed - hence we need to manually set it here.
                handle.GetSlice()->GetRowTable(softDeletedDocumentRow.GetRank()).SetBit(handle.GetSlice()->GetSliceBuffer(),
                                                                                        softDeletedDocumentRow.GetIndex(),
                                                                                        handle.GetIndex());
                TestAssert(IsDocumentActive(handle, softDeletedDocumentRow));

                // For the purpose of this test, do not expire the last DocIndex, as it will trigger a Slice
                // recycling in the background thread and we don't want to manage the lifetime of that thread
                // in this test.
                if (i != c_sliceCapacity - 1)
                {
                    handle.Expire();

                    TestAssert(!IsDocumentActive(handle, softDeletedDocumentRow));
                }
            }
        }


        // Fills up a Slice full of commited documents and returns a pointer to 
        // this Slice.
        Slice* FillUpSlice(Shard& shard, DocIndex sliceCapacity)
        {
            Slice* slice = nullptr;
            for (DocIndex i = 0; i < sliceCapacity; ++i)
            {
                const DocumentHandleInternal handle = shard.AllocateDocument();

                if (slice == nullptr)
                {
                    slice = handle.GetSlice();
                }

                slice->CommitDocument(handle.GetIndex());
            }

            return slice;
        }


        // Test to verify that expiring the last document in a Slice, schedules it for
        // recycling.
        TestCase(ExpireTriggersRecycleTest)
        {
            static const DocIndex c_sliceCapacity = Row::DocumentsInRank0Row(1);

            static const std::vector<RowIndex> rowCounts = { 100, 0, 0, 200, 0, 0, 300 };
            std::shared_ptr<ITermTable const> termTable(new MockTermTable(0));

            DocumentDataSchema schema;

            const size_t sliceBufferSize = Shard::InitializeDescriptors(nullptr, 
                                                                        c_sliceCapacity, 
                                                                        schema, 
                                                                        *termTable);
            std::unique_ptr<ISliceBufferAllocator> sliceBufferAllocator(
                new TrackingSliceBufferAllocator(sliceBufferSize));
            TrackingSliceBufferAllocator& trackingAllocator
                = dynamic_cast<TrackingSliceBufferAllocator&>(*sliceBufferAllocator);

            IndexWrapper index(c_sliceCapacity, termTable, schema, sliceBufferAllocator);
            Shard& shard = index.GetShard();

            TestEqual(trackingAllocator.GetInUseBuffersCount(), 0);

            {
                // Create a Slice and expire all documents. Verify it got recycled.
                Slice* currentSlice = FillUpSlice(shard, c_sliceCapacity);

                TestEqual(trackingAllocator.GetInUseBuffersCount(), 1);

                // Expire all documents in the Slice. This should decrement ref count to 1.
                // The slice is still not recycled since there is one reference holder.
                for (DocIndex i = 0; i < c_sliceCapacity; ++i)
                {
                    DocumentHandleInternal handle(currentSlice, i);
                    handle.Expire();
                }

                // Verify that the Slice got recycled.
                TestEqual(trackingAllocator.GetInUseBuffersCount(), 0);
            }

            {
                // This time, simulate that there is another reference holder of the Slice.
                Slice* currentSlice = FillUpSlice(shard, c_sliceCapacity);

                TestEqual(trackingAllocator.GetInUseBuffersCount(), 1);

                // Simulate another reference holder of the slice, such as backup writer.
                Slice::IncrementRefCount(currentSlice);

                // The Slice should not be recycled since there are 2 reference holders.
                TestEqual(trackingAllocator.GetInUseBuffersCount(), 1);

                // Expire all documents in the Slice. This should decrement ref count to 1.
                // The slice is still not recycled since there is one reference holder.
                for (DocIndex i = 0; i < c_sliceCapacity; ++i)
                {
                    DocumentHandleInternal handle(currentSlice, i);
                    handle.Expire();
                }

                // Verify that the Slice did not get recycled.
                TestEqual(trackingAllocator.GetInUseBuffersCount(), 1);

                // Decrement the last ref count, Slice should be scheduled for recycling.
                Slice::DecrementRefCount(currentSlice);
                TestEqual(trackingAllocator.GetInUseBuffersCount(), 0);
            }
        }
    }
}