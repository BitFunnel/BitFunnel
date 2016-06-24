#include "stdafx.h"

#include <memory>

#include "BitFunnel/Factories.h"
#include "BitFunnel/IBlockAllocator.h"
#include "ExpectException.h"
#include "LoggerInterfaces/Logging.h"
#include "ThrowingLogger.h"
#include "SuiteCpp/UnitTest.h"


namespace BitFunnel
{
    namespace BlockAllocatorUnitTest
    {
        TestCase(BasicTest)
        {
            static const size_t c_blockSize = 8;
            static const size_t c_totalBlockCount = 3;

            std::unique_ptr<IBlockAllocator> allocator(
                Factories::CreateBlockAllocator(c_blockSize, c_totalBlockCount));

            TestEqual(c_blockSize, allocator->GetBlockSize());

            unsigned __int64 * const block1 = allocator->AllocateBlock();
            *block1 = 123;

            unsigned __int64 * const block2 = allocator->AllocateBlock();
            TestAssert(block1 != block2);
            *block2 = 456;

            unsigned __int64 * const block3 = allocator->AllocateBlock();
            TestAssert(block2 != block3);
            TestAssert(block1 != block3);
            *block3 = 789;

            // No more blocks available.
            ExpectException([&] () { allocator->AllocateBlock(); });

            // Release a block and try again.
            allocator->ReleaseBlock(block1);

            // block4 sould be same as block1.
            unsigned __int64* const block4 = allocator->AllocateBlock();
            TestAssert(block1 == block4);

            ExpectException([&] () { allocator->AllocateBlock(); });

            allocator->ReleaseBlock(block2);
            allocator->ReleaseBlock(block3);

            unsigned __int64* const block5 = allocator->AllocateBlock();
            TestAssert(block5 != nullptr);

            unsigned __int64* const block6 = allocator->AllocateBlock();
            TestAssert(block6 != nullptr);

            // Release blocks out of order.
            allocator->ReleaseBlock(block6);
            allocator->ReleaseBlock(block5);

            unsigned __int64* const block7 = allocator->AllocateBlock();
            TestAssert(block7 != nullptr);

            unsigned __int64* const block8 = allocator->AllocateBlock();
            TestAssert(block8 != nullptr);
        }


        TestCase(RoundUpTest)
        {
            static const size_t c_blockSize = 3;
            static const size_t c_totalBlockCount = 4;

            std::unique_ptr<IBlockAllocator> allocator(
                Factories::CreateBlockAllocator(c_blockSize, c_totalBlockCount));

            // Requested block size should be rounded up to 8.
            TestEqual(8, allocator->GetBlockSize());

            // Should be able to get 4 blocks out of the allocator.
            allocator->AllocateBlock();
            allocator->AllocateBlock();
            allocator->AllocateBlock();
            allocator->AllocateBlock();
        }


        TestCase(ReleaseWrongBlockTest)
        {
            ThrowingLogger logger;
            Logging::RegisterLogger(&logger);

            static const size_t c_blockSize = 16;
            static const size_t c_totalBlockCount = 4;

            std::unique_ptr<IBlockAllocator> allocator(
                Factories::CreateBlockAllocator(c_blockSize, c_totalBlockCount));

            unsigned __int64 * block = allocator->AllocateBlock();

            // Cannot release block which is outside of our range.
            ExpectException([&]() { 
                allocator->ReleaseBlock(block + c_blockSize * c_totalBlockCount / sizeof(unsigned __int64));
            });

            ExpectException([&]() { 
                allocator->ReleaseBlock(block - 1);
            });

            // Cannot release a block which is not positioned at a multiple 
            // of the blockSize.
            ExpectException([&]() { 
                allocator->ReleaseBlock(block + 1);
            });

            allocator->ReleaseBlock(block);
            allocator->ReleaseBlock(block + 2);
            allocator->ReleaseBlock(block + 4);
        }
    }
}