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

#include <memory>
#include <new>

#include "BitFunnel/Exceptions.h"
#include "LoggerInterfaces/Logging.h"
#include "PrivateHeapAllocator.h"


namespace BitFunnel
{
    PrivateHeapAllocator::PrivateHeapAllocator()
    {
        Initialize(1024 * 1024);
    }


    PrivateHeapAllocator::PrivateHeapAllocator(size_t bufferSize)
    {
        Initialize(bufferSize);
    }


    // Free memory and recycle the heap so it can be reused
    void PrivateHeapAllocator::Reset()
    {
        m_start = m_next = m_buffer.get();
    }


    // Allocate a block of memory of specified size from an heap allocator,
    // or from overflow allocator if the size is greater than maxArenaAlloc.
    void* PrivateHeapAllocator::Allocate(size_t size)
    {
        if (m_next + size >= m_end)
        {
            RecoverableError error("PrivateHeapAllocator::Allocate: out of space.");
            throw error;
        }

        void* p = m_next;
        m_next += size;
        return p;
    }


    // "Free" memory allocated by the allocator.
    void PrivateHeapAllocator::Deallocate(void* /*p*/)
    {
        // Intentional nop.
    }


    size_t PrivateHeapAllocator::MaxSize() const
    {
        return m_end - m_start;
    }

    void PrivateHeapAllocator::Initialize(size_t bufferSize)
    {
        m_buffer.reset(new char[bufferSize]);
        Reset();
        m_end = m_start + bufferSize;
    }

    //PrivateHeapAllocatorFactory::~PrivateHeapAllocatorFactory()
    //{
    //    for (unsigned i = 0 ; i < m_freeAllocators.size(); ++i)
    //    {
    //        delete m_freeAllocators[i];
    //    }
    //}


    //Allocators::IAllocator& PrivateHeapAllocatorFactory::CreateAllocator()
    //{
    //    std::lock_guard<std::mutex> lock(m_lock);
    //    if (m_freeAllocators.size() > 0)
    //    {
    //        Allocators::IAllocator* allocator = m_freeAllocators.back();
    //        m_freeAllocators.pop_back();
    //        return *allocator;
    //    }
    //    else
    //    {
    //        Allocators::IAllocator* allocator = new PrivateHeapAllocator();
    //        return *allocator;
    //    }
    //}


    //void PrivateHeapAllocatorFactory::ReleaseAllocator(Allocators::IAllocator& allocator)
    //{
    //    std::lock_guard<std::mutex> lock(m_lock);
    //    m_freeAllocators.push_back(&allocator);
    //}
}
