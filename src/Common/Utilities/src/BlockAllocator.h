#pragma once


#include <mutex>

#include "BitFunnel/Allocators/IBlockAllocator.h"
#include "AlignedBuffer.h"

namespace BitFunnel
{
    //*************************************************************************
    //
    // BlockAllocator is an implementation of the IBlockAllocator that 
    // allocates the entire pool of the requested number of blocks at 
    // construction and never releases it until destruction. Internally the 
    // pool is aligned to c_byteAlignment. The list of available blocks is 
    // stored as a linked list where the pointer to the next item is stored in 
    // the beginning of the block itself. If this value is nullptr, this is the
    // last block. m_freeListHead points to the first available block, or 
    // nullptr if there are no available blocks.
    // Requesting a block when there are none available results in an exception.
    //
    // DESIGN NOTE: The main usage of this allocator is for the RowTable rows 
    // which operate on quadwords. Therefore the allocator's pointers are 
    // uint64_t * and all blocks coming from the allocator are properly
    // aligned to use for matcher.
    //
    //*************************************************************************
    class BlockAllocator : public IBlockAllocator
    {
    public:
        // Constructs an allocator with given block size and the total number
        // of blocks in the pool.
        // Requested blockSize will be rounded up to the next multiple of
        // c_byteAlignment.
        BlockAllocator(size_t blockSize, size_t totalBlockCount);

        //
        // IBlockAllocator API.
        //
        virtual uint64_t* AllocateBlock() override;
        virtual void ReleaseBlock(uint64_t*) override;
        virtual size_t GetBlockSize() const override;

    private:
        // Byte alignment of the allocated blocks.
        static const unsigned c_log2ByteAlignment = 3;
        static const unsigned c_byteAlignment = 1U << c_log2ByteAlignment;

        const size_t m_blockSize;
        const size_t m_totalPoolSize;

        // Lock protecting operations on the pool.
        std::mutex m_lock;

        // Underlying pool of memory blocks.
        AlignedBuffer m_pool;

        // A pointer to the first available block.
        uint64_t * m_freeListHead;
    };
}
