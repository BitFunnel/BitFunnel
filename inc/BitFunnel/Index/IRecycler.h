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
