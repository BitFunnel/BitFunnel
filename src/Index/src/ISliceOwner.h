#pragma once

#include "BitFunnel/IInterface.h"

namespace BitFunnel
{
    class Slice;

    class ISliceOwner : public IInterface
    {
    public:
        virtual void RecycleSlice(Slice& slice) = 0;
        virtual void ReleaseSliceBuffer(void* sliceBuffer) = 0;
    };
}
