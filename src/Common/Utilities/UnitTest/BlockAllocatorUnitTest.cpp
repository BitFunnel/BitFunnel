#include <memory>

#include "gtest/gtest.h"

#include "BitFunnel/Allocators/Factories.h"
#include "BitFunnel/Allocators/IBlockAllocator.h"
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
                Factories::CreateBlockAllocator(c_blockSize, c_totalBlockCount));

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
                Factories::CreateBlockAllocator(c_blockSize, c_totalBlockCount));

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
                Factories::CreateBlockAllocator(c_blockSize, c_totalBlockCount));

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
