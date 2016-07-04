#pragma once

#include <memory>
#include <vector>

#include "BitFunnel/IInterface.h"
#include "BitFunnel/NonCopyable.h"
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
    // TODO: Other naming ideas:
    // RecyclingSlice, RecyclingSlices, SliceBeingRecycled, SlicePendingRecycle.
    // TODO: Consider moving to Shard.cpp, as it is the only consumer of the class.
    class SliceListChangeRecyclable : public IRecyclable
    {
    public:
        SliceListChangeRecyclable(Slice* slice,
                                  std::vector<void*> const * sliceBuffers,
                                  ITokenManager& tokenManager);

        //
        // IRecyclable API.
        //
        virtual bool TryRecycle() override;

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

        // Adds a resource to the list for recycling.
        // Recycler takes ownership of the resource.
        virtual void ScheduleRecyling(std::unique_ptr<IRecyclable>& resource) override;
    private:
    };
}
