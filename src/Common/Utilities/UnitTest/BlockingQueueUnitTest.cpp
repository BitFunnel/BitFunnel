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

#include "BlockingQueue.h"
#include "ThreadManager.h"
#include "gtest/gtest.h"

namespace BitFunnel
{
    namespace BlockingQueueUnitTest
    {
        class ProducerConsumerThread : public IThreadBase
        {
        public:
            ProducerConsumerThread(bool isProducer,
                bool shutdownOk,
                uint64_t itemCount,
                BlockingQueue<uint64_t>& queue);

            void EntryPoint();

            bool IsProducer() const;
            uint64_t GetItemsProcessed() const;

        private:
            bool m_isProducer;
            bool m_shutdownOk;
            uint64_t m_itemCount;
            uint64_t m_itemsProcessed;
            BlockingQueue<uint64_t>& m_queue;
        };


        void RunTest1(unsigned queueLength,
                      unsigned producerCount,
                      unsigned itemsPerProducer,
                      unsigned consumerCount,
                      bool shutdown);


        //*********************************************************************
        TEST(BlockingQueueUnitTest, Comprehensive)
        {
            RunTest1(100, 10, 1000, 10, false);         // Lots of readers and writers.
            RunTest1(100, 10, 869, 3, false);           // Reader:Writer ratio not integer.
            RunTest1(100, 10, 867, 1, false);           // Many writers, one reader.
            RunTest1(100, 2, 10000, 10, false);         // Few writers, many readers.
            RunTest1(1, 2, 10000, 10, false);           // length-1 queue, many readers
            RunTest1(1, 10, 1000, 1, false);            // length-1 queue, many writers
            RunTest1(100000, 10, 1000, 10, false);      // Try a case where enqueue will never block.
            RunTest1(1, 1, 1000001, 1003, true);        // Shutdown before finished.
            RunTest1(1, 1003, 1000, 1, true);           // Shutdown before finished.
        }

        //*********************************************************************
        //
        // DESIGN NOTE: This test only checks that the number of items enqueued
        // is equal to the number of items dequeued. It does not verify ordering
        // of items. There are two rationale. The first is that BlockingQueue is
        // based on std::dequeue which is assumed to preserve ordering. The
        // second is that it is hard to design a multithreaded reader/writer
        // test the verifies ordering without taking locks that might have the
        // unintended consequence of synchronizing the threads in a way that
        // could hide threading errors. This test is mainly to check the
        // correctness of the semaphores and events and the locking mechanism
        // around the std::dequeue.
        //
        // Create a blocking queue with length = 'queueLength'. Then start
        // a set of producer threads that place items in the queue and a set of
        // consumer threads that remove items from the queue. Run until all
        // threads have finished. Then verify the results.
        //
        //*********************************************************************
        void RunTest1(unsigned queueLength,
                      unsigned producerCount,
                      unsigned itemsPerProducer,
                      unsigned consumerCount,
                      bool shutdown)
        {
            BlockingQueue<uint64_t> queue(queueLength);

            std::vector<ProducerConsumerThread*> producerConsumers;
            std::vector<IThreadBase*> threads;

            // Create producer threads.
            for (unsigned i = 0 ; i < producerCount; ++i)
            {
                producerConsumers.push_back(new ProducerConsumerThread(true, shutdown, itemsPerProducer, queue));
                threads.push_back(producerConsumers.back());
            }
            unsigned totalItems = itemsPerProducer * producerCount;
            unsigned itemsPerConsumer = totalItems / consumerCount;

            // Create consumer threads.
            unsigned consumerItems = 0;
            for (unsigned i = 0 ; i < consumerCount; ++i)
            {
                if (i < consumerCount - 1)
                {
                    producerConsumers.push_back(new ProducerConsumerThread(false, shutdown, itemsPerConsumer, queue));
                }
                else
                {
                    // Last consumer gets all remaining items which may differ
                    // from itemsPerConsumer because of roundoff.
                    producerConsumers.push_back(new ProducerConsumerThread(false, shutdown, totalItems - consumerItems, queue));
                }
                threads.push_back(producerConsumers.back());
                consumerItems += itemsPerConsumer;
            }

            // Start the threads and wait for them to complete.
            ThreadManager threadManager(threads);
            if (shutdown)
            {
                queue.Shutdown();
                threadManager.WaitForThreads();
                return;
            }
            threadManager.WaitForThreads();

            // Verify the results.
            uint64_t totalItemsProduced = 0;
            uint64_t totalItemsConsumed = 0;
            for (unsigned i = 0 ; i < producerConsumers.size(); ++i)
            {
                if (producerConsumers[i]->IsProducer())
                {
                    totalItemsProduced += producerConsumers[i]->GetItemsProcessed();
                }
                else
                {
                    totalItemsConsumed += producerConsumers[i]->GetItemsProcessed();
                }
            }

            ASSERT_EQ(totalItemsProduced, totalItemsConsumed);
            ASSERT_EQ(totalItemsProduced, totalItems);

            for (size_t i = 0 ; i < threads.size(); ++i)
            {
                delete threads[i];
            }
        }


        //*********************************************************************
        //
        // ProducerConsumerThread
        //
        //*********************************************************************
        ProducerConsumerThread::ProducerConsumerThread(bool isProducer,
            bool shutdownOk,
            uint64_t itemCount,
            BlockingQueue<uint64_t>& queue)
            : m_isProducer(isProducer),
            m_shutdownOk(shutdownOk),
            m_itemCount(itemCount),
            m_itemsProcessed(0),
            m_queue(queue)
        {
        }


        void ProducerConsumerThread::EntryPoint()
        {
            uint64_t i = 0;
            for ( ; i < m_itemCount; ++i)
            {
                if (!m_shutdownOk)
                {
                    if (m_isProducer)
                    {
                        ASSERT_TRUE(m_queue.TryEnqueue(i));
                    }
                    else
                    {
                        uint64_t value;
                        ASSERT_TRUE(m_queue.TryDequeue(value));
                    }
                }
                ++m_itemsProcessed;
                // No sleeps because we're trying to maximize the chance of threads
                // interfering with each other.
            }
        }


        uint64_t ProducerConsumerThread::GetItemsProcessed() const
        {
            return m_itemsProcessed;
        }


        bool ProducerConsumerThread::IsProducer() const
        {
            return m_isProducer;
        }
    }
}
