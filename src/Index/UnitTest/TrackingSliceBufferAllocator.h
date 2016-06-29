#pragma once

#include <set>

#include "ISliceBufferAllocator.h"
#include "Mutex.h"

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
        mutable Mutex m_lock;
        std::set<void*> m_allocatedBuffers;
        const size_t m_blockSize;
    };
}