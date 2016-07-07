#pragma once

#include <memory>                   // std::shared_ptr is a parameter.

#include "BitFunnel/IInterface.h"
#include "BitFunnel/NonCopyable.h"

namespace BitFunnel
{
    class IRecyclable;

    //*************************************************************************
    //
    // Abstract class or interface for classes which handle recycling of items
    // of the IRecyclable type which have been scheduled for recycling.
    //
    //*************************************************************************
    class IRecycler : public IInterface
    {
    public:
        // Run thread that actually recycles things.
        virtual void Run() = 0;

        // Adds a resource to the list for recycling.
        // Recycler takes ownership of the resource.
        virtual void ScheduleRecyling(std::unique_ptr<IRecyclable>& resource) = 0;

        virtual void Shutdown() = 0;
    };
}
