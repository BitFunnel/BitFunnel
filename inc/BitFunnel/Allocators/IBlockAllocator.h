#pragma once

namespace BitFunnel
{
    //************************************************************************* 
    //
    // IBlockAllocator is an abstract class or interface for classes that are
    // used to allocate blocks of memory of the same size out of a shared pool
    // of memory of a fixed size. The size of the block and the number of 
    // blocks in the pool are immutable once the allocator is created. 
    // Allocated blocks are guaranteed to be byte aligned for use with the 
    // matching engine. To achieve that, the size of the block will be rounded
    // up to the next aligned value.
    //
    // DESIGN NOTE: IBlockAllocator does not protect from calling 
    // ReleaseBlock multiple times for the same block before it is allocated
    // again.
    //
    // All methods are thread safe.
    //
    //************************************************************************* 
    class IBlockAllocator
    {
    public:
        virtual ~IBlockAllocator() {};

        // Allocates a block of memory from a pool. If no block is available
        // for allocation, this method throws.
        virtual uint64_t* AllocateBlock() = 0;

        // Returns the block back to the pool. 
        virtual void ReleaseBlock(uint64_t* block) = 0;

        // Returns the size of the blocks in the pool.
        virtual size_t GetBlockSize() const = 0;
    };
}
