#include "stdafx.h"

#include <set>

#include "LockGuard.h"
#include "TrackingSliceBufferAllocator.h"
#include "SuiteCpp/UnitTest.h"


namespace BitFunnel
{
    TrackingSliceBufferAllocator::TrackingSliceBufferAllocator(size_t blockSize)
        : m_blockSize(blockSize)
    {
    }


    size_t TrackingSliceBufferAllocator::GetInUseBuffersCount() const
    {
        LockGuard lock(m_lock);

        return m_allocatedBuffers.size();
    }


    void* TrackingSliceBufferAllocator::Allocate(size_t byteSize)
    {
        LockGuard lock(m_lock);

        TestAssert(byteSize == m_blockSize);

        void* sliceBuffer = malloc(byteSize);
        m_allocatedBuffers.insert(sliceBuffer);

        return sliceBuffer;
    }


    void TrackingSliceBufferAllocator::Release(void* buffer)
    {
        LockGuard lock(m_lock);

        auto it = m_allocatedBuffers.find(buffer);
        TestAssert(it != m_allocatedBuffers.end());

        free(buffer);

        m_allocatedBuffers.erase(it);
    }


    size_t TrackingSliceBufferAllocator::GetSliceBufferSize() const
    {
        return m_blockSize;
    }
}