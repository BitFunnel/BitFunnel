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

#include <mutex>
#include <unordered_set>

#include "ISliceBufferAllocator.h"

namespace BitFunnel
{
    // Test implementation of the ISliceBufferAllocator which tracks the list of buffers
    // allocated.
    class TrackingSliceBufferAllocator : public ISliceBufferAllocator
    {
    public:
        TrackingSliceBufferAllocator(size_t blockSize);

        size_t GetInUseBuffersCount() const;

        virtual void* Allocate(size_t byteSize) override;
        virtual void Release(void* buffer) override;
        virtual size_t GetSliceBufferSize() const override;

    private:
        mutable std::mutex m_lock;
        std::unordered_set<void*> m_allocatedBuffers;
        const size_t m_blockSize;
    };
}
