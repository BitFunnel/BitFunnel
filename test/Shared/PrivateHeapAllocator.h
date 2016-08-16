#pragma once

#include <vector>
#include <windows.h>

#include "BitFunnelAllocatorInterfaces/IAllocator.h"
#include "BitFunnelAllocatorInterfaces/IAllocatorFactory.h"
#include "Mutex.h"


namespace BitFunnel
{
    class PrivateHeapAllocator : public Allocators::IAllocator
    {
    public:
        // constructor
        PrivateHeapAllocator();
        PrivateHeapAllocator(size_t minBuffer);

        // destructor
        ~PrivateHeapAllocator();

        // Free memory and recycle the heap so it can be reused
        void Reset();
        
        // Allocate a block of memory of specified size from an heap allocator,
        // or from overflow allocator if the size is greater than maxArenaAlloc.
        void* Allocate(size_t size);
        
        // "Free" memory allocated by the allocator.
        void Deallocate(void* p);

        size_t MaxSize() const;

    private:
        // handle to the kernel heap
        HANDLE m_heap;
        size_t m_minBuffer;
    };


    class PrivateHeapAllocatorFactory : public Allocators::IAllocatorFactory
    {
    public:
        ~PrivateHeapAllocatorFactory();

        //
        // IAllocatorFactory methods.
        //

        // Returns an allocator from m_freeAllocators if any are available.
        // Otherwise, creates and returns a new allocator. This method is
        // threadsafe.
        Allocators::IAllocator& CreateAllocator();

        // Returns an allocator to m_freeAllocators. Note that allocators are
        // not actually deleted until this class' destructor is called.
        // This method is threadsafe.
        void ReleaseAllocator(Allocators::IAllocator& allocator);

    private:
        // m_lock protects multithreaded access to m_freeAllocators.
        Mutex m_lock;

        // m_freeAllocators is a vector of allocators that have been returned
        // to the factory via the ReleaseAllocator() method. CreateAllocator()
        // always attempts to reuse an allocator from m_freeAllocators before
        // creating a new one.
        std::vector<Allocators::IAllocator*> m_freeAllocators;

    };
}


