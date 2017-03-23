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
#include "TrackingSliceBufferAllocator.h"


namespace BitFunnel
{
    namespace ShardTest
    {
        TEST(Shard, AddRemoveSlices)
        {
            auto recycler = Factories::CreateRecycler();
            auto background = std::async(std::launch::async, &IRecycler::Run, recycler.get());

            auto tokenManager = Factories::CreateTokenManager();
            auto termTable = Factories::CreateTermTable();
            termTable->Seal();

            DocumentDataSchema docDataSchema;

            const size_t blockSize =
                GetMinimumBlockSize(docDataSchema, *termTable);

            std::unique_ptr<TrackingSliceBufferAllocator>
                trackingAllocator(new TrackingSliceBufferAllocator(blockSize));

            ShardId anyShardId = 0;
            Shard shard(anyShardId,
                        *recycler,
                        *tokenManager,
                        *termTable,
                        docDataSchema,
                        *trackingAllocator,
                        blockSize);

            auto sliceCapacity = shard.GetSliceCapacity();
            ASSERT_GT(sliceCapacity, 0u);
            Slice* currentSlice = nullptr;
            std::vector<Slice*> slices;
            const size_t c_numSlices = 4;

            for (DocIndex i = 0; i < sliceCapacity * c_numSlices; ++i)
            {
                const DocumentHandleInternal h = shard.AllocateDocument(i);
                if (i % sliceCapacity == 0)
                {
                    EXPECT_NE(&h.GetSlice(), currentSlice);
                    slices.push_back(&h.GetSlice());
                }
                currentSlice = &h.GetSlice();
            }

            EXPECT_EQ(trackingAllocator->GetInUseBuffersCount(),
                      c_numSlices);

            for (auto const & slice : slices)
            {
                ASSERT_NE(slice, nullptr);
                for (DocIndex i = 0; i < sliceCapacity; ++i)
                {
                    slice->CommitDocument();
                }
            }

            // TODO: check that the slice pointer points to something we can
            // write to?

            EXPECT_EQ(trackingAllocator->GetInUseBuffersCount(),
                      c_numSlices);

            for (auto & slice : slices)
            {
                for (DocIndex i = 0; i < sliceCapacity; ++i)
                {
                    // TODO: try to recycle non-expired Slice.
                    slice->ExpireDocument();
                }
                shard.RecycleSlice(*slice);
            }

            while(trackingAllocator->GetInUseBuffersCount() != 0u) {}

            // TODO: try to recycle non-existent slice.

            tokenManager->Shutdown();
            recycler->Shutdown();
            background.wait();
        }
    }
}
