#include <exception>
#include <memory>

#include "BitFunnel/Allocators/Factories.h"
#include "BitFunnel/Allocators/IBlockAllocator.h"
#include "BlockAllocator.h"
#include "LoggerInterfaces/Logging.h"
#include "Rounding.h"

namespace BitFunnel
{

    std::unique_ptr<IBlockAllocator>
        Factories::
        CreateBlockAllocator(size_t blockSize, size_t totalBlockCount)
    {
        return std::unique_ptr<IBlockAllocator>(
            new BlockAllocator(blockSize, totalBlockCount));
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
        LogAssertB(m_blockSize > 0, "m_blockSize of 0.");
        LogAssertB(totalBlockCount > 0, "totalBlockCount of 0.");

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

        m_freeListHead = static_cast<uint64_t*>(m_pool.GetBuffer());
    }


    uint64_t * BlockAllocator::AllocateBlock()
    {
        std::lock_guard<std::mutex> lock(m_lock);

        if (m_freeListHead == nullptr)
        {
            throw std::runtime_error("Out of memory");
        }

        uint64_t * block = m_freeListHead;
        m_freeListHead = reinterpret_cast<uint64_t*>(*m_freeListHead);

        return block;
    }


    void BlockAllocator::ReleaseBlock(uint64_t * block)
    {
        // Casting to char * for pointer arithmetics.
        char const * blockReturned = reinterpret_cast<char const *>(block);

        // Checking that the returned block belongs to our range.
        char const * bufferStart = static_cast<char const *>(m_pool.GetBuffer());
        LogAssertB(blockReturned >= bufferStart,
                   "ReleaseBlock out of range (< bufferStart).");
        LogAssertB(blockReturned < bufferStart + m_totalPoolSize,
                   "ReleaseBlock out of range (past end)).");

        // Block offset relative to the beginning of the pool should be a 
        // multiple of m_blockSize;
        LogAssertB(((blockReturned - bufferStart) % m_blockSize) == 0,
                   "Block offset (relative to begining of pool not a multiple of blockSize");

        // Add the block to the head of the free list
        uint64_t** blockPtr = reinterpret_cast<uint64_t**>(block);

        std::lock_guard<std::mutex> lock(m_lock);

        *blockPtr = m_freeListHead;

        m_freeListHead = block;
    }


    size_t BlockAllocator::GetBlockSize() const
    {
        return m_blockSize;
    }
}
