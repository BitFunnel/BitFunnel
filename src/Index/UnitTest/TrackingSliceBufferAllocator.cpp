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

        if (byteSize != m_blockSize)
        {
            // TODO: remove.
            throw byteSize;
        }
        EXPECT_EQ(byteSize, m_blockSize);

        void* sliceBuffer = malloc(byteSize);
        m_allocatedBuffers.insert(sliceBuffer);

        return sliceBuffer;
    }


    void TrackingSliceBufferAllocator::Release(void* buffer)
    {
        std::lock_guard<std::mutex> lock(m_lock);

        auto it = m_allocatedBuffers.find(buffer);
        EXPECT_NE(it, m_allocatedBuffers.end());

        free(buffer);

        m_allocatedBuffers.erase(it);
    }


    size_t TrackingSliceBufferAllocator::GetSliceBufferSize() const
    {
        return m_blockSize;
    }
}
