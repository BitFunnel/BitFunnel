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


#include <iostream>  // TODO: remove.

#include <mutex>
#include <unordered_set>

#include "gtest/gtest.h"

#include "TrackingSliceBufferAllocator.h"


namespace BitFunnel
{
    TrackingSliceBufferAllocator::TrackingSliceBufferAllocator(size_t blockSize)
        : m_blockSize(blockSize)
    {
    }


    size_t TrackingSliceBufferAllocator::GetInUseBuffersCount() const
    {
        std::lock_guard<std::mutex> lock(m_lock);

        return m_allocatedBuffers.size();
    }


    void* TrackingSliceBufferAllocator::Allocate(size_t byteSize)
    {
        std::lock_guard<std::mutex> lock(m_lock);

        EXPECT_EQ(byteSize, m_blockSize);

        void* sliceBuffer = malloc(byteSize);
        m_allocatedBuffers.insert(sliceBuffer);

        std::cout << "Allocating " << byteSize << " bytes at "
                  << std::hex << sliceBuffer
                  << std::dec << std::endl;

        return sliceBuffer;
    }


    void TrackingSliceBufferAllocator::Release(void* buffer)
    {
        std::lock_guard<std::mutex> lock(m_lock);

        const auto it = m_allocatedBuffers.find(buffer);
        ASSERT_NE(it, m_allocatedBuffers.end());

        free(buffer);

        m_allocatedBuffers.erase(it);
    }


    size_t TrackingSliceBufferAllocator::GetSliceBufferSize() const
    {
        return m_blockSize;
    }
}
