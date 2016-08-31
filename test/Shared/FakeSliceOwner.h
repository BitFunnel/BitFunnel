#pragma once

#include "BitFunnel/NonCopyable.h"
#include "ISliceOwner.h"

namespace BitFunnel
{
    class ISliceBufferAllocator;
    class Slice;

    class FakeSliceOwner : private NonCopyable, public ISliceOwner
    {
    public:
        FakeSliceOwner(ISliceBufferAllocator& sliceBufferAllocator);

        void RecycleSlice(Slice& slice);
        void ReleaseSliceBuffer(void* sliceBuffer);

    private:
        ISliceBufferAllocator& m_sliceBufferAllocator;
    };
}
