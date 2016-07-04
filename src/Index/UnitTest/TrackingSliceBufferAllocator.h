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
