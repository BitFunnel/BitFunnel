#include <chrono> // Used for temporary blocking recycle.
#include <iostream> // TODO: remove.
#include <thread> // Used for temporary blocking recycle.

#include "BitFunnel/Token.h"
#include "LoggerInterfaces/Logging.h"
#include "Recycler.h"
#include "Slice.h"

namespace BitFunnel
{
    //*************************************************************************
    //
    // Recycler.
    //
    //*************************************************************************
    // TODO: put this arbitrary 100 constant somewhere
    Recycler::Recycler()
        : m_queue (std::unique_ptr<BlockingQueue<IRecyclable*>>
                   (new BlockingQueue<IRecyclable*>(100))),
          m_shutdown (false)
    {
    }


    void Recycler::Run()
    {
        while (!m_shutdown)
        {
            IRecyclable* item;
            // false indicates queue shutdown.
            if (!m_queue->TryDequeue(item))
            {
                return;
            }
            LogAssertB(item, "null IRecycable item.");
            item->Recycle();
        }
    }

    void Recycler::ScheduleRecyling(std::unique_ptr<IRecyclable>& resource)
    {
        auto ptr = resource.release();
        LogAssertB(m_queue->TryEnqueue(ptr),
                   "ScheduleRecycling called on queue that's shutting down.");
    }


    void Recycler::Shutdown()
    {
        m_queue->Shutdown();
    }


    //*************************************************************************
    //
    // SliceListChangeRecyclable.
    //
    //*************************************************************************
    SliceListChangeRecyclable::SliceListChangeRecyclable(Slice* slice,
                                                         std::vector<void*> const * sliceBuffers,
                                                         ITokenManager& tokenManager)
        : m_slice(slice),
          m_sliceBuffers(sliceBuffers),
          m_tokenTracker(tokenManager.StartTracker())
    {
    }


    void SliceListChangeRecyclable::Recycle()
    {
        std::cout << "Recycle: waiting for token tracker completion.\n";
        m_tokenTracker->WaitForCompletion();
        std::cout << "Recycle: Recycling.\n";
        if (m_slice != nullptr)
        {
            // Deleting a Slice invokes its destructor which returns its
            // slice buffer to the allocator.
            delete m_slice;
        }

        delete m_sliceBuffers;
    }
}
