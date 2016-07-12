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

#include <memory>

#include "BitFunnel/Allocators/IAllocator.h"
#include "BitFunnel/Utilities/IBlockAllocator.h"
#include "BitFunnel/NonCopyable.h"
#include "ISliceBufferAllocator.h"

namespace BitFunnel
{
    //*************************************************************************
    //
    // Implementation of the ISliceBufferAllocator which pre-allocates a fixed
    // number of blocks of the same byte size and re-uses them for Slices.
    // Slices adjusts their capacity based on the size of the buffer.
    //
    // Allocate method expects only a well-known value of the buffer size,
    // otherwise it throws.
    //
    // This class is thread safe.
    //
    //*************************************************************************
    class SliceBufferAllocator : public ISliceBufferAllocator, NonCopyable
    {
    public:
        // Creates a SliceBufferAllocator which uses IBlockAllocator under the
        // hood to allocate and release blocks of the same byte size.
        SliceBufferAllocator(size_t blockSize, size_t blockCount);

        //
        // ISliceBufferAllocator API.
        //
        virtual void* Allocate(size_t byteSize) override;
        virtual void Release(void* buffer) override;
        virtual size_t GetSliceBufferSize() const override;

    private:

        // Block allocator which hands out the blocks of the fixed size.
        std::unique_ptr<IBlockAllocator> const m_blockAllocator;
    };
}
