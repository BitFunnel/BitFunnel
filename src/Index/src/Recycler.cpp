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


#include <chrono> // Used for temporary blocking recycle.
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
            // TODO: fix leak. Items inside destructing queue will get leaked.
            // Should probably manage this with a unique_ptr.
            delete item;
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
        m_tokenTracker->WaitForCompletion();
        if (m_slice != nullptr)
        {
            // Deleting a Slice invokes its destructor which returns its
            // slice buffer to the allocator.
            delete m_slice;
        }

        delete m_sliceBuffers;
    }
}
