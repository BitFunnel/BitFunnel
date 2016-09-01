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

#pragma once

#include <cstddef>
#include <mutex>
#include <vector>

#include "BitFunnel/Allocators/IAllocator.h"
//#include "BitFunnel/Allocators/IAllocatorFactory.h"


namespace BitFunnel
{
    class PrivateHeapAllocator : public IAllocator
    {
    public:
        PrivateHeapAllocator();
        PrivateHeapAllocator(size_t bufferSize);

        //
        // IAllocator methods
        //

        // Allocate a block of memory of specified size from an heap allocator,
        // or from overflow allocator if the size is greater than maxArenaAlloc.
        virtual void* Allocate(size_t size) override;

        // "Free" memory allocated by the allocator.
        virtual void Deallocate(void* p) override;

        virtual size_t MaxSize() const override;

        // Free memory and recycle the heap so it can be reused
        virtual void Reset() override;

    private:
        void Initialize(size_t bufferSize);

        std::unique_ptr<char []> m_buffer;
        char* m_start;
        char* m_end;
        char* m_next;
    };


    //class PrivateHeapAllocatorFactory : public IAllocatorFactory
    //{
    //public:
    //    ~PrivateHeapAllocatorFactory();

    //    //
    //    // IAllocatorFactory methods.
    //    //

    //    // Returns an allocator from m_freeAllocators if any are available.
    //    // Otherwise, creates and returns a new allocator. This method is
    //    // threadsafe.
    //    Allocators::IAllocator& CreateAllocator();

    //    // Returns an allocator to m_freeAllocators. Note that allocators are
    //    // not actually deleted until this class' destructor is called.
    //    // This method is threadsafe.
    //    void ReleaseAllocator(Allocators::IAllocator& allocator);

    //private:
    //    // m_lock protects multithreaded access to m_freeAllocators.
    //    std::mutex m_lock;

    //    // m_freeAllocators is a vector of allocators that have been returned
    //    // to the factory via the ReleaseAllocator() method. CreateAllocator()
    //    // always attempts to reuse an allocator from m_freeAllocators before
    //    // creating a new one.
    //    std::vector<IAllocator*> m_freeAllocators;
    //};
}
