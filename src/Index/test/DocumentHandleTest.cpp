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
#include "BitFunnel/Index/Helpers.h"
#include "BitFunnel/Index/IRecycler.h"
#include "BitFunnel/Index/ISliceBufferAllocator.h"
#include "BitFunnel/Index/ITermTable.h"
#include "BitFunnel/Index/Token.h"
#include "BitFunnel/Utilities/Factories.h"
#include "DocumentDataSchema.h"
#include "IndexUtils.h"
#include "Shard.h"
#include "Slice.h"
#include "TrackingSliceBufferAllocator.h"

// TODO: put test scaffolding into a class. This hasn't been done because we're
// not sure what we'll want to be able to vary for tests.

namespace BitFunnel
{

    struct FixedSizeBlob0
    {
        uint64_t m_field1;
        double m_field2;
        uint32_t m_field3;
        float m_field4;
    };


    TEST(DocumentHandle, Blobs)
    {
        auto recycler = Factories::CreateRecycler();
        auto background = std::async(std::launch::async, &IRecycler::Run, recycler.get());

        auto tokenManager = Factories::CreateTokenManager();

        // TermTable setup.  This is done so we can add some postings, to see if
        // postings somehow interfere with blobs.
        auto termTable = Factories::CreateTermTable();

        const size_t termCount = 5;
        const size_t explicitRowCount = 100;
        const size_t adhocRowCount = 200;

        // Start hash at 1000 to be well above the hashes reserved for system rows and facts.
        const Term::Hash c_firstHash = 1000ull;

        // Add a bunch of Explicit terms.
        for (size_t i = 0; i < termCount; ++i)
        {
            Term::Hash hash = i + c_firstHash;

            // For this test, the number of rows for each term is based on
            // its hash.
            termTable->OpenTerm();
            for (size_t r = 0; r <= (i % 3); ++r)
            {
                termTable->AddRowId(RowId(0, 0, r));
            }
            termTable->CloseTerm(hash);
        }

        termTable->SetRowCounts(0, explicitRowCount, adhocRowCount);
        termTable->SetFactCount(0);
        termTable->Seal();

        DocumentDataSchema docDataSchema;
        // TODO: check more than one of each type of blob.
        const VariableSizeBlobId variableSizeBlobId =
            docDataSchema.RegisterVariableSizeBlob();
        const FixedSizeBlobId fixedSizeBlobId =
            docDataSchema.RegisterFixedSizeBlob(sizeof(FixedSizeBlob0));

        const size_t blockSize =
            GetMinimumBlockSize(docDataSchema, *termTable);

        std::unique_ptr<TrackingSliceBufferAllocator>
            trackingAllocator(new TrackingSliceBufferAllocator(blockSize));

        Shard shard(*recycler, *tokenManager, *termTable, docDataSchema, *trackingAllocator, blockSize);
        auto sliceCapacity = shard.GetSliceCapacity();
        Slice* currentSlice = nullptr;
        std::vector<Slice*> slices;
        const size_t c_numSlices = 1;
        std::vector<DocumentHandleInternal> handles;
        std::vector<void*> variableBlobs;

        for (DocIndex i = 0; i < sliceCapacity * c_numSlices; ++i)
        {
                const DocumentHandleInternal h = shard.AllocateDocument(i);
                handles.push_back(h);
                if (i % sliceCapacity == 0)
                {
                    EXPECT_NE(h.GetSlice(), currentSlice);
                    slices.push_back(h.GetSlice());
                }
                currentSlice = h.GetSlice();
        }


        // Set blobs to arbitrary values.
        double doubleVal = 3.0;
        float floatVal = 5.0;
        for (size_t i = 0; i < sliceCapacity * c_numSlices; ++i)
        {
            const size_t blobSize = 1+i;
            void* blob = handles[i].AllocateVariableSizeBlob(variableSizeBlobId, blobSize);
            variableBlobs.push_back(blob);
            memset(blob, 0xFF & i, blobSize);

            FixedSizeBlob0& fixedSizeBlobValue =
                *static_cast<FixedSizeBlob0*>
                (handles[i].GetFixedSizeBlob(fixedSizeBlobId));
            fixedSizeBlobValue.m_field1 = 2+i;
            fixedSizeBlobValue.m_field2 = doubleVal++;
            fixedSizeBlobValue.m_field3 = static_cast<uint32_t>(4+i);
            fixedSizeBlobValue.m_field4 = floatVal++;
        }


        // Create some artibrary postings.
        for (size_t i = 0; i < termCount; ++i)
        {
            Term::Hash hash = i + c_firstHash;
            // hash, streamId, idf, gramSize.
            Term term(hash, 0, 10, 1);
            for (auto & handle : handles)
            {
                handle.AddPosting(term);
            }
        }


        // Check blob values.
        doubleVal = 3.0;
        floatVal = 5.0;
        for (size_t i = 0; i < sliceCapacity * c_numSlices; ++i)
        {
            uint8_t* blob = reinterpret_cast<uint8_t*>(variableBlobs[i]);
            for (size_t j = 0; j < 1+i; ++j)
            {
                EXPECT_EQ(*blob, 0xFF & i);
                ++blob;
            }

            FixedSizeBlob0& fixedSizeBlobValue =
                *static_cast<FixedSizeBlob0*>
                (handles[i].GetFixedSizeBlob(fixedSizeBlobId));
            EXPECT_EQ(fixedSizeBlobValue.m_field1, 2+i);
            EXPECT_EQ(fixedSizeBlobValue.m_field2, doubleVal++);
            EXPECT_EQ(fixedSizeBlobValue.m_field3 , 4+i);
            EXPECT_EQ(fixedSizeBlobValue.m_field4, floatVal++);
        }

        // Expire documents. Not necessary for test, just to avoid leaking
        // Slices, which causes valgrind to complain.
        for (auto const & slice : slices)
        {
            ASSERT_NE(slice, nullptr);
            for (DocIndex i = 0; i < sliceCapacity; ++i)
            {
                slice->CommitDocument();
            }
        }

        for (auto & handle : handles)
        {
            handle.Expire();
        }

        while(trackingAllocator->GetInUseBuffersCount() != 0u) {}

        tokenManager->Shutdown();
        recycler->Shutdown();
        background.wait();
    }
}
