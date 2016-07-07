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


#include <iostream>

#include "LoggerInterfaces/Logging.h"
#include "Shard.h"
#include "Slice.h"


namespace BitFunnel
{
    Slice::Slice(Shard& shard)
        : m_shard(shard),
          m_temporaryNextDocIndex(0U),
          m_capacity(shard.GetSliceCapacity()),
          m_refCount(1),
          m_buffer(shard.AllocateSliceBuffer()),
          m_unallocatedCount(shard.GetSliceCapacity()),
          m_commitPendingCount(0),
          m_expiredCount(0)
    {
        Initialize();
    }


    Shard& Slice::GetShard() const
    {
        return m_shard;
    }


    Slice::~Slice()
    {
        try
        {
            // GetDocTable().Cleanup(m_buffer);
            m_shard.ReleaseSliceBuffer(m_buffer);
        }
        catch (...)
        {
            LogB(Logging::Error, "Slice", "Exception caught in Slice::~Slice()","");
        }
    }


    bool Slice::TryAllocateDocument(size_t& index)
    {
        std::lock_guard<std::mutex> lock(m_docIndexLock);

        if (m_unallocatedCount == 0)
        {
                return false;
        }

        index = m_capacity - m_unallocatedCount;
        m_unallocatedCount--;
        m_commitPendingCount++;

        return true;
    }

    bool Slice::CommitDocument()
    {
        std::lock_guard<std::mutex> lock(m_docIndexLock);
        LogAssertB(m_commitPendingCount > 0,
                   "CommitDocument with m_commitPendingCount == 0");

        --m_commitPendingCount;

        return (m_unallocatedCount + m_commitPendingCount) == 0;
    }


    /* static */
    void Slice::DecrementRefCount(Slice* slice)
    {
        const unsigned newRefCount = --(slice->m_refCount);
        std::cout << "DecrementRefCount " << newRefCount << std::endl;
        if (newRefCount == 0)
        {
            slice->GetShard().RecycleSlice(*slice);
        }
    }


    bool Slice::ExpireDocument()
    {
        std::lock_guard<std::mutex> lock(m_docIndexLock);

        // Cannot expire more than what was committed.
        const DocIndex committedCount =
            m_capacity - m_unallocatedCount - m_commitPendingCount;
        LogAssertB(m_expiredCount < committedCount,
                   "Slice expired more documents than committed.");

        m_expiredCount++;

        return m_expiredCount == m_capacity;
    }


    /* static */
    // TODO: why do we have these two methods which are basically identical?
    Slice* Slice::GetSliceFromBuffer(void* sliceBuffer, ptrdiff_t slicePtrOffset)
    {
        return Slice::GetSlicePointer(sliceBuffer, slicePtrOffset);
    }


    /* static */
    Slice*& Slice::GetSlicePointer(void* sliceBuffer, ptrdiff_t slicePtrOffset)
    {
        char* slicePtr = reinterpret_cast<char*>(sliceBuffer) + slicePtrOffset;
        return *reinterpret_cast<Slice**>(slicePtr);
    }


    /* static */
    void Slice::IncrementRefCount(Slice* slice)
    {
        ++(slice->m_refCount);
    }


    void Slice::Initialize()
    {
        // Place a pointer to a Slice in the last bytes of the SliceBuffer.
        Slice*& slicePtr = GetSlicePointer(m_buffer, m_shard.GetSlicePtrOffset());
        slicePtr = this;
    }


    bool Slice::IsExpired() const
    {
        // TODO: is this lock_guard  really necessary?
        // It seems like this could be removed if m_expireCount were atomic.
        std::lock_guard<std::mutex> lock(m_docIndexLock);

        return m_expiredCount == m_capacity;
    }


    void* Slice::GetSliceBuffer() const
    {
        return m_buffer;
    }
}
