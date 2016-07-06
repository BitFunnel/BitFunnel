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

#include <atomic>
#include <condition_variable> // for std::condition_variable
#include <deque>
#include <mutex>

namespace BitFunnel
{
    //*************************************************************************
    //
    // BlockingQueue<T> implements a threadsafe queue with a fixed capacity.
    // Attempts to dequeue when empty and enqueue when full will block the
    // caller.
    //
    //*************************************************************************
    template <typename T>
    class BlockingQueue
    {
    public:
        // Construct a BlockingQueue with a specified capacity.
        BlockingQueue(unsigned capacity);

        // Destroys the BlockingQueue and its associated events and semaphores.
        ~BlockingQueue();

        void Shutdown();

        // Returns true if item is successfully enqueued.
        // Returns false if the item cannot be enqueued (unsually because queue
        // is at capacity or shutting down).
        bool TryEnqueue(T value);

        // Blocks the caller until a value is available or the queue is
        // shutdown. Returns true if an item was successfully dequeued. Returns
        // false if the timeout expired or the queue was shut down.
        bool TryDequeue(T& value);

    private:
        std::condition_variable m_enqueueCond;
        std::condition_variable m_dequeueCond;
        std::mutex m_lock;

        size_t m_capacity;
        std::atomic<bool> m_shutdown;

        std::deque<T> m_queue;
    };


    //*************************************************************************
    //
    // Implementation of BlockingQueue<T>
    //
    //*************************************************************************
    template <typename T>
    BlockingQueue<T>::BlockingQueue(unsigned capacity)
        : m_capacity(capacity),
          m_shutdown(false)
    {
    }


    template <typename T>
    BlockingQueue<T>::~BlockingQueue()
    {
        Shutdown();
    }


    template <typename T>
    void BlockingQueue<T>::Shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(m_lock);
            m_shutdown = true;
        }
        m_dequeueCond.notify_all();
        m_enqueueCond.notify_all();
    }


    template <typename T>
    bool BlockingQueue<T>::TryEnqueue(T value)
    {
        std::unique_lock<std::mutex> lock(m_lock);
        // TODO: get rid of lambda. Mhop looked at this at one point and VC++
        // generates a lot of allocations when we use the lambda.
        m_enqueueCond.wait
            (lock,
             [&]{ return m_queue.size() < m_capacity || m_shutdown; });
        if (m_shutdown)
        {
            return false;
        }
        m_queue.push_back(value);
        m_dequeueCond.notify_one();
        return true;
    }


    template <typename T>
    bool BlockingQueue<T>::TryDequeue(T& value)
    {
        std::unique_lock<std::mutex> lock(m_lock);
        m_dequeueCond.wait
            (lock,
             [&]{ return !m_queue.empty() || m_shutdown; });
        if (m_shutdown)
        {
            return false;
        }
        value = m_queue.front();
        m_queue.pop_front();
        m_enqueueCond.notify_one();
        return true;
    }
}
