#pragma once

#include "IAllocator.h"

namespace BitFunnel
{
    namespace Allocators
    {
        //*************************************************************************
        //
        // IAllocatorFactory is an abstract base class or interface for classes
        // that create and recycle memory allocators. Classes the implement
        // IAllocatorFactory may employ any recycling strategy and may choose
        // whether or not to be threadsafe.
        //
        //*************************************************************************
        class IAllocatorFactory
        {
        public:
            virtual ~IAllocatorFactory() {};

            // Obtains and returns an allocator. The allocator may be created on
            // demand or it may be reclaimed from a pool of released allocators.
            virtual IAllocator& CreateAllocator() = 0;

            // Returns an allocator to the factory for reuse or destruction. Note
            // that the allocator must be one that was provided by the
            // CreateAllocator() method on the same IAllocatorFactory instance.
            virtual void ReleaseAllocator(IAllocator& allocator) = 0;
        };
    }
}
