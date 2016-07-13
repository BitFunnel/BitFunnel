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

#include <memory>
#include <vector>

#include "BitFunnel/IInterface.h"
#include "BitFunnel/NonCopyable.h"
#include "BlockingQueue.h"
#include "IRecyclable.h"
#include "IRecycler.h"

namespace BitFunnel
{
    class ITokenManager;
    class ITokenTracker;
    class Slice;
    class SliceBufferAllocator;

    // Class which represents a recycling logic which happens after a list of
    // slices was changed - either a new Slice was added to the list, or a
    // Slice was removed from the list. The structure which represent a new
    // new slice list is swapped in using the interlocked pointer exchange, and
    // the old structure can be deleted after draining all the thread which
    // might be still using it.
    //
    // Two main scenarios of using the class:
    // 1. Adding a new slice. In this case, this class is handed the old vector
    //    of pointers to Slices which existed before the change. It will delete
    //    the vector after draining the queries.
    // 2. Deleting a Slice. In addition to the old vector of pointers, in this
    //    case it is also handed a pointer to a Slice being removed. Recycling
    //    involves deleting the vector and returning the Slice back to its
    //    allocator and deleting the resources it held.
    //
    // Uses token system to determine when the consumers of the resource have
    // exited.
    //
    // TODO: Consider moving to Shard.cpp, as it is the only consumer of the class.
    class DeferredSliceListDelete : public IRecyclable
    {
    public:
        DeferredSliceListDelete(Slice* slice,
                                  std::vector<void*> const * sliceBuffers,
                                  ITokenManager& tokenManager);

        //
        // IRecyclable API.
        //
        virtual void Recycle() override;

    private:
        Slice* m_slice;
        std::vector<void*> const * m_sliceBuffers;

        // Token tracker which is associated with this recyclable.
        // When all of the tokens which it tracks, have been removed from
        // circulation, m_slice and m_sliceList can be recycled.
        std::shared_ptr<ITokenTracker> m_tokenTracker;
    };


    //*************************************************************************
    //
    // Class which implements a list of IRecyclable instances which have been
    // scheduled for recycling. Instances of IRecyclable indicate that they
    // can be recycled by returning true from their CanRecycle() method.
    //
    // Maintains a queue of objects that were dropped for recyling and polls
    // them to check if they can be recycled by checking their CanRecycle(),
    // and it if returns true, calls their OnRecycle().
    //
    //*************************************************************************
    class Recycler : public IRecycler, NonCopyable
    {
    public:
        Recycler();

        void Run() override;

        void Shutdown() override;

        // Adds a resource to the list for recycling.
        // Recycler takes ownership of the resource.
        virtual void
            ScheduleRecyling(std::unique_ptr<IRecyclable>& resource) override;
    private:
        // TODO: we should log of this queue fills to the point of blocking on
        // enqueue. That's an unexpected condition.
        std::unique_ptr<BlockingQueue<IRecyclable*>> m_queue;

        std::atomic<bool> m_shutdown;
    };
}
