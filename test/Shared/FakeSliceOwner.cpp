#include "BitFunnel/Index/ISliceBufferAllocator.h"
#include "FakeSliceOwner.h"
#include "Slice.h"

namespace BitFunnel
{
    FakeSliceOwner::FakeSliceOwner(ISliceBufferAllocator& sliceBufferAllocator)
        : m_sliceBufferAllocator(sliceBufferAllocator)
    {}

    void FakeSliceOwner::RecycleSlice(Slice& slice)
    {
        // TODO: should we fix this by changing the API?  Calling delete on the
        // address of a reference is a gross violation of normal C++
        // conventions, but we only do this in the Fake. OTOH, the real code
        // does this indirectly in a way.
        delete &slice;
    }

    void FakeSliceOwner::ReleaseSliceBuffer(void* sliceBuffer)
    {
        m_sliceBufferAllocator.Release(sliceBuffer);
    }
}
