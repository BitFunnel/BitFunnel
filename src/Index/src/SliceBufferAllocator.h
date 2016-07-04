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
