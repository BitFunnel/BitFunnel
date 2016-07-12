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

#include "BitFunnel/IInterface.h"

namespace BitFunnel
{
    //*************************************************************************
    //
    // ISliceBufferAllocator is an abstract base class or interface for classes
    // that implement an allocator which is used to manage memory allocation
    // for Slices in the Index.
    //
    // When a Slice is created, its memory buffer will be allocated from this
    // allocator, and when a Slice is recycled, its memory buffer will be
    // returned to this allocator for re-use. Implementations of the
    // ISliceBufferAllocator may either pre-allocate a fixed number of blocks
    // of the same size and the Slices will adjust their capacities based on
    // the size of the block, or the allocator may allow allocating a fixed set
    // of buffer sizes, one for each for each shard.
    //
    // DESIGN NOTE: When a buffer is returned to the pool, it is zero
    // initialized in order to speed up creation of Slice from this buffer.
    //
    //*************************************************************************
    class ISliceBufferAllocator : public IInterface
    {
    public:
        // Allocates a buffer for a Slice and returns a pointer to it.
        // Implementors may restrict byteSize to a pre-defined set of values,
        // or even require a single value to be used for all slices in the
        // Index.
        virtual void* Allocate(size_t byteSize) = 0;

        // Returns the allocator when a Slice is being recycled back to the
        // pool for re-use. Buffer is zero initialized upon return.
        virtual void Release(void* buffer) = 0;

        // Returns the size of the single slice buffer.
        // DESIGN NOTE: Initially we only support a single value for all byte
        // sizes across Shards, and Shards choose their capacity based on
        // the buffer size, hence the allocator exposes this value. Going
        // forward, ISliceBufferAllocator may support multiple buffer sizes,
        // one for each shard. At this point this method may not be applicable
        // and can be removed.
        virtual size_t GetSliceBufferSize() const = 0;
    };
}
