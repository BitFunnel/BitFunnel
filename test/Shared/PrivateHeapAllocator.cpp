#include "stdafx.h"

#include <new>

#include "PrivateHeapAllocator.h"
#include "LockGuard.h"
#include "LoggerInterfaces/Logging.h"


namespace BitFunnel
{
    PrivateHeapAllocator::PrivateHeapAllocator()
        : m_minBuffer(1024 * 1024)
    {
        m_heap = HeapCreate(0, m_minBuffer, 0);
        LogAssertB(m_heap != nullptr);
    }


    PrivateHeapAllocator::PrivateHeapAllocator(size_t minBuffer)
        : m_minBuffer(minBuffer)
    {
        m_heap = HeapCreate(0, m_minBuffer, 0);
        LogAssertB(m_heap != nullptr);
    }


    PrivateHeapAllocator::~PrivateHeapAllocator()
    {
        if (m_heap != nullptr)
        {
            HeapDestroy(m_heap);
        }
    }


    // Free memory and recycle the heap so it can be reused
    void PrivateHeapAllocator::Reset()
    {
        if (m_heap != nullptr)
        {
            HeapDestroy(m_heap);
        }
        m_heap = HeapCreate(0, m_minBuffer, 0);
    }
    

    // Allocate a block of memory of specified size from an heap allocator,
    // or from overflow allocator if the size is greater than maxArenaAlloc.
    void* PrivateHeapAllocator::Allocate(size_t size)
    {
        void* p = HeapAlloc(m_heap, 0, size);
        if (p == nullptr)
        {
            throw std::bad_alloc();
        }
        return p;
    }


    // "Free" memory allocated by the allocator.
    void PrivateHeapAllocator::Deallocate(void* p)
    {
        HeapFree(m_heap, 0, p);
    }


    size_t PrivateHeapAllocator::MaxSize() const
    {
        return m_minBuffer;
    }


    PrivateHeapAllocatorFactory::~PrivateHeapAllocatorFactory()
    {
        for (unsigned i = 0 ; i < m_freeAllocators.size(); ++i)
        {
            delete m_freeAllocators[i];
        }
    }


    Allocators::IAllocator& PrivateHeapAllocatorFactory::CreateAllocator()
    {
        LockGuard lockGuard(m_lock);
        if (m_freeAllocators.size() > 0)
        {
            Allocators::IAllocator* allocator = m_freeAllocators.back();
            m_freeAllocators.pop_back();
            return *allocator;
        }
        else
        {
            Allocators::IAllocator* allocator = new PrivateHeapAllocator();
            return *allocator;
        }
    }


    void PrivateHeapAllocatorFactory::ReleaseAllocator(Allocators::IAllocator& allocator)
    {
        LockGuard lockGuard(m_lock);
        m_freeAllocators.push_back(&allocator);
    }
}
