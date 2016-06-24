#include "stdafx.h"

#include <exception>
#include <memory>

#include "BitFunnel/Factories.h"
#include "BitFunnel/IBlockAllocator.h"
#include "BlockAllocator.h"
#include "LockGuard.h"
#include "LoggerInterfaces/Logging.h"
#include "Rounding.h"

namespace BitFunnel
{
    std::unique_ptr<IBlockAllocator> Factories::CreateBlockAllocator(size_t blockSize, 
                                                                     size_t totalBlockCount)
    {
        return std::unique_ptr<IBlockAllocator>(new BlockAllocator(blockSize, 
                                                                   totalBlockCount));
    }


    BlockAllocator::BlockAllocator(size_t blockSize, size_t totalBlockCount)
        : m_blockSize(RoundUp(blockSize, c_byteAlignment)),
          m_totalPoolSize(m_blockSize * totalBlockCount),
          m_pool(m_totalPoolSize, c_log2ByteAlignment)
    {
        // TODO: technically, one can create an allocator with a size = 0 which
        // would simply throw on the first allocation. This would allow not 
        // having any special handling on the client side where allocation is 
        // not needed. However given that this is not a public class and we 
        // know exactly its usage, let's not support this scenario now and 
        // re-visit it in future if needed.
        LogAssertB(m_blockSize > 0);
        LogAssertB(totalBlockCount > 0);

        char * currentBlock = static_cast<char *>(m_pool.GetBuffer());

        for (size_t block = 0; block < totalBlockCount; ++block)
        {
            char** nextBlockPtr = reinterpret_cast<char**>(currentBlock);
            currentBlock += m_blockSize;

            if (block != totalBlockCount - 1)
            {
                *nextBlockPtr = currentBlock;
            }
            else
            {
                *nextBlockPtr = nullptr;
            }
        }

        m_freeListHead = static_cast<unsigned __int64*>(m_pool.GetBuffer());
    }


    unsigned __int64 * BlockAllocator::AllocateBlock()
    {
        LockGuard lock(m_lock);

        if (m_freeListHead == nullptr)
        {
            throw std::exception("Out of memory");
        }

        unsigned __int64 * block = m_freeListHead;
        m_freeListHead = reinterpret_cast<unsigned __int64*>(*m_freeListHead);

        return block;
    }


    void BlockAllocator::ReleaseBlock(unsigned __int64 * block)
    {
        // Casting to char * for pointer arithmetics.
        char const * blockReturned = reinterpret_cast<char const *>(block);

        // Checking that the returned block belongs to our range.
        char const * bufferStart = static_cast<char const *>(m_pool.GetBuffer());
        LogAssertB(blockReturned >= bufferStart);
        LogAssertB(blockReturned < bufferStart + m_totalPoolSize);

        // Block offset relative to the beginning of the pool should be a 
        // multiple of m_blockSize;
        LogAssertB(((blockReturned - bufferStart) % m_blockSize) == 0);

        // Add the block to the head of the free list
        unsigned __int64** blockPtr = reinterpret_cast<unsigned __int64**>(block);

        LockGuard lock(m_lock);

        *blockPtr = m_freeListHead;

        m_freeListHead = block;
    }


    size_t BlockAllocator::GetBlockSize() const
    {
        return m_blockSize;
    }
}