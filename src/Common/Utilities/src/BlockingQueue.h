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

#include <deque>

namespace BitFunnel
{
    //*************************************************************************
    //
    // BlockingQueueBase
    //
    // Base class for queues that use semphores to track the amount of free
    // and used slots.
    //
    // THREAD SAFETY: With the exception of the constructor and destructor,
    // all methods are thread safe.
    //
    //*************************************************************************
    class BlockingQueueBase
    {
    public:
        BlockingQueueBase(unsigned capacity);
        ~BlockingQueueBase();

        // Shuts down the queue. Immediately returns control to all threads 
        // suspended on TryDeque() and TryEnqueue().
        void Shutdown();

        // Returns true if the queue is in the process of shutting down.
        bool IsShuttingDown() const;

    protected:
        // Waits up to timeoutInMS for an item to become available for dequeue.
        // Returns true if an item becomes available for the calling thread.
        // At this point, the item is reserved for the caller and the caller
        // must process the item and then invoke CompleteDequeue(). Returns
        // false if no item becomes available in the timeout period. Also
        // returns false if the queue is shutting down.
        bool TryDequeue(unsigned timeoutInMS);


        // Waits up to timeoutInMS for space to become available for enqueue.
        // Returns true if a space becomes available for the calling thread.
        // At this point, the space is reserved for the caller and the caller
        // must process use the space and then invoke CompleteEnqueue(). Returns
        // false if no space becomes available in the timeout period. Also
        // returns false if the queue is shutting down.
        bool TryEnqueue(unsigned timeoutInMS);

        // Companion to TryDequeue(). Must be called after TryDequeue()
        // returns true. This method updates the dequeue semaphore which
        // tracks the amount of free space in the queue.
        void CompleteDequeue();

        // Companion to TryEnqueue(). Must be called after TryEnqueue()
        // returns true. This method updates the enqueue semaphore which
        // tracks the number of items in the queue.
        void CompleteEnqueue();

    private:
        HANDLE m_shutdownEvent;
        HANDLE m_enqueueSemaphore;
        HANDLE m_dequeueSemaphore;
    };

    //*************************************************************************
    //
    // BlockingQueue<T> implements a threadsafe queue with a fixed capacity.
    // Attempts to dequeue when empty and enqueue when full will block the
    // caller.
    //
    //*************************************************************************
    template <typename T>
    class BlockingQueue : public BlockingQueueBase
    {
    public:
        // Construct a BlockingQueue with a specified capacity.
        BlockingQueue(unsigned capacity);

        // Destroys the BlockingQueue and its associated events and semaphores.
        ~BlockingQueue();

        // Returns true if item is successfully enqueued.
        // Returns false if the item cannot be enqueued (unsually because queue
        // is at capacity or shutting down).
        bool TryEnqueue(T value, unsigned timeoutInMS);

        // Blocks the caller until a value is available, the queue is shutdown,
        // or the timeout expires. Returns true if an item was successfully
        // dequeued. Returns false if the timeout expired or the queue was shut
        // down.
        bool TryDequeue(T& value, unsigned timeoutInMS);

    private:
        // Protects m_queue operations.
        std::mutex m_lock;

        std::deque<T> m_queue;
    };


    //*************************************************************************
    //
    // Implementation of BlockingQueue<T>
    //
    //*************************************************************************
    template <typename T>
    BlockingQueue<T>::BlockingQueue(unsigned capacity)
        : BlockingQueueBase(capacity)
    {
    }


    template <typename T>
    BlockingQueue<T>::~BlockingQueue()
    {
        Shutdown();
    }


    template <typename T>
    bool BlockingQueue<T>::TryEnqueue(T value, unsigned timeoutInMS)
    {
        if (BlockingQueueBase::TryEnqueue(timeoutInMS))
        {
            std::lock_guard<std::mutex> lock(m_lock);
            m_queue.push_back(value);
            CompleteEnqueue();
            return true;
        }
        else
        {
            return false;
        }
    }


    template <typename T>
    bool BlockingQueue<T>::TryDequeue(T& value, unsigned timeoutInMS)
    {
        if (BlockingQueueBase::TryDequeue(timeoutInMS))
        {
            std::lock_guard<std::mutex> lock(m_lock);
            value = m_queue.front();
            m_queue.pop_front();
            CompleteDequeue();
            return true;
        }
        else
        {
            return false;
        }
    }
}
