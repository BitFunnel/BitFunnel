#include <chrono> // Used for temporary blocking recycle.
#include <iostream> // TODO:
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
    Recycler::Recycler()
    {
    }


    void Recycler::ScheduleRecyling(std::unique_ptr<IRecyclable>& resource)
    {
        // TODO: replace this with something that inserts item into
        // AsyncQueue?
        for (;;)
        {
            if (resource->TryRecycle())
            {
                break;
            }

            // TODO: remove this hack.
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1ms);
        }
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


    bool SliceListChangeRecyclable::TryRecycle()
    {
        if (!m_tokenTracker->IsComplete())
        {
            return false;
        }

        if (m_slice != nullptr)
        {
            // Deleting a Slice invokes its destructor which returns its
            // slice buffer to the allocator.
            delete m_slice;
        }

        delete m_sliceBuffers;

        return true;
    }
}
