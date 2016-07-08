#include <inttypes.h>
#include <iostream>  // TODO: remove.

#include "BitFunnel/Utilities/Factories.h"
#include "LoggerInterfaces/Logging.h"
#include "SliceBufferAllocator.h"

namespace BitFunnel
{
    SliceBufferAllocator::SliceBufferAllocator(size_t blockSize, size_t blockCount)
        : m_blockAllocator(Factories::CreateBlockAllocator(blockSize, blockCount))
    {
    }


    void* SliceBufferAllocator::Allocate(size_t byteSize)
    {
        // TODO: should this be more strict and allow only exactly sized blocks?
        LogAssertB(m_blockAllocator->GetBlockSize() <= byteSize,
                   "Allocate byteSize < block size.");

        return m_blockAllocator->AllocateBlock();
    }


    void SliceBufferAllocator::Release(void* buffer)
    {
        m_blockAllocator->ReleaseBlock(reinterpret_cast<uint64_t*>(buffer));
    }


    size_t SliceBufferAllocator::GetSliceBufferSize() const
    {
        return m_blockAllocator->GetBlockSize();
    }
}
