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
