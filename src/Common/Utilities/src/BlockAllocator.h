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


#pragma once


#include <mutex>  // For std::mutex.

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
