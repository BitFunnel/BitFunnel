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


#include <memory>

#include "gtest/gtest.h"

#include "BitFunnel/Utilities/Factories.h"
#include "BitFunnel/Utilities/IBlockAllocator.h"
#include "LoggerInterfaces/Logging.h"
#include "ThrowingLogger.h"


namespace BitFunnel
{
    namespace BlockAllocatorUnitTest
    {
        TEST(BasicTest, Trivial)
        {
            static const size_t c_blockSize = 8;
            static const size_t c_totalBlockCount = 3;

            std::unique_ptr<IBlockAllocator> allocator(
                Factories::CreateBlockAllocator(c_blockSize,
                                                c_totalBlockCount));

            EXPECT_EQ(c_blockSize, allocator->GetBlockSize());

            uint64_t * const block1 = allocator->AllocateBlock();
            *block1 = 123;

            uint64_t * const block2 = allocator->AllocateBlock();
            EXPECT_NE(block1, block2);
            *block2 = 456;

            uint64_t * const block3 = allocator->AllocateBlock();
            EXPECT_NE(block2, block3);
            EXPECT_NE(block1, block3);
            *block3 = 789;

            // No more blocks available.
            // TODO: replace with specific exception type once BitFunnel
            // exception types are ported.
            EXPECT_ANY_THROW(allocator->AllocateBlock());

            // Release a block and try again.
            allocator->ReleaseBlock(block1);

            // block4 sould be same as block1.
            uint64_t* const block4 = allocator->AllocateBlock();
            EXPECT_EQ(block1, block4);

            EXPECT_ANY_THROW(allocator->AllocateBlock());

            allocator->ReleaseBlock(block2);
            allocator->ReleaseBlock(block3);

            uint64_t* const block5 = allocator->AllocateBlock();
            EXPECT_NE(block5, nullptr);

            uint64_t* const block6 = allocator->AllocateBlock();
            EXPECT_NE(block6, nullptr);

            // Release blocks out of order.
            allocator->ReleaseBlock(block6);
            allocator->ReleaseBlock(block5);

            uint64_t* const block7 = allocator->AllocateBlock();
            EXPECT_NE(block7, nullptr);

            uint64_t* const block8 = allocator->AllocateBlock();
            EXPECT_NE(block8, nullptr);
        }


        TEST(RoundUpTest, Trivial)
        {
            static const size_t c_blockSize = 3;
            static const size_t c_totalBlockCount = 4;

            std::unique_ptr<IBlockAllocator> allocator(
                Factories::CreateBlockAllocator(c_blockSize,
                                                c_totalBlockCount));

            // Requested block size should be rounded up to 8.
            EXPECT_EQ(8u, allocator->GetBlockSize());

            // Should be able to get 4 blocks out of the allocator.
            allocator->AllocateBlock();
            allocator->AllocateBlock();
            allocator->AllocateBlock();
            allocator->AllocateBlock();
        }


        TEST(ReleaseWrongBlockTest, Trivial)
        {
            ThrowingLogger logger;
            Logging::RegisterLogger(&logger);

            static const size_t c_blockSize = 16;
            static const size_t c_totalBlockCount = 4;

            std::unique_ptr<IBlockAllocator> allocator(
                Factories::CreateBlockAllocator(c_blockSize,
                                                c_totalBlockCount));

            uint64_t * block = allocator->AllocateBlock();

            // Cannot release block which is outside of our range.
            EXPECT_ANY_THROW(
                allocator->ReleaseBlock(
                    block + c_blockSize * c_totalBlockCount / sizeof(uint64_t));
            );

            EXPECT_ANY_THROW(allocator->ReleaseBlock(block - 1));

            // Cannot release a block which is not positioned at a multiple
            // of the blockSize.
            EXPECT_ANY_THROW(allocator->ReleaseBlock(block + 1));

            allocator->ReleaseBlock(block);
            allocator->ReleaseBlock(block + 2);
            allocator->ReleaseBlock(block + 4);
        }
    }
}
