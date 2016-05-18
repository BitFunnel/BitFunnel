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

#include "ThreadsafeCounter.h"
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
            ProducerConsumerThread(int id,
                bool isProducer, 
                unsigned __int64 itemCount,
                BlockingQueue<unsigned __int64>& queue);

            void EntryPoint();

            bool IsProducer() const;
            unsigned __int64 GetItemsProcessed() const;

        private:
            int m_id;
            bool m_isProducer;
            unsigned __int64 m_itemCount;
            unsigned __int64 m_itemsProcessed;
            BlockingQueue<unsigned __int64>& m_queue;
        };


        void RunTest1(unsigned queueLength,
            unsigned producerCount,
            unsigned itemsPerProducer,
            unsigned consumerCount);


        //*********************************************************************
        TEST(BlockingQueueUnitTest, Comprehensive)
        {
            RunTest1(100, 10, 1000, 10);         // Lots of readers and writers.
            RunTest1(100, 10, 869, 3);           // Reader:Writer ratio not integer.
            RunTest1(100, 10, 867, 1);           // Many writers, one reader.
            RunTest1(100, 2, 10000, 10);         // Few writers, many readers.
            RunTest1(1, 2, 10000, 10);           // length-1 queue, many readers
            RunTest1(1, 10, 1000, 1);            // length-1 queue, many writers
            RunTest1(100000, 10, 1000, 10);      // Try a case where enqueue will never block.
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
            unsigned consumerCount)
        {
            BlockingQueue<unsigned __int64> queue(queueLength);

            std::vector<ProducerConsumerThread*> producerConsumers;
            std::vector<IThreadBase*> threads;

            // Create producer threads.
            for (unsigned i = 0 ; i < producerCount; ++i)
            {
                producerConsumers.push_back(new ProducerConsumerThread(i, true, itemsPerProducer, queue));
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
                    producerConsumers.push_back(new ProducerConsumerThread(i, false, itemsPerConsumer, queue));
                }
                else
                {
                    // Last consumer gets all remaining items which may differ
                    // from itemsPerConsumer because of roundoff.
                    producerConsumers.push_back(new ProducerConsumerThread(i, false, totalItems - consumerItems, queue));
                }
                threads.push_back(producerConsumers.back());
                consumerItems += itemsPerConsumer;
            }

            // Start the threads and wait for them to complete.
            ThreadManager threadManager(threads);
            threadManager.WaitForThreads(INFINITE);

            // Verify the results.
            unsigned __int64 totalItemsProduced = 0;
            unsigned __int64 totalItemsConsumed = 0;
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

            for (int i = 0 ; i < threads.size(); ++i)
            {
                delete threads[i];
            }
        }


        //*********************************************************************
        //
        // ProducerConsumerThread
        //
        //*********************************************************************
        ProducerConsumerThread::ProducerConsumerThread(int id,
            bool isProducer,
            unsigned __int64 itemCount,
            BlockingQueue<unsigned __int64>& queue)
            : m_id(id),
            m_isProducer(isProducer),
            m_itemCount(itemCount),
            m_itemsProcessed(0),
            m_queue(queue)
        {
        }


        void ProducerConsumerThread::EntryPoint()
        {
            unsigned __int64 i = 0;
            for ( ; i < m_itemCount; ++i)
            {
                if (m_isProducer)
                {
                    ASSERT_TRUE(m_queue.TryEnqueue(i, INFINITE));
                }
                else
                {
                    unsigned __int64 value;
                    ASSERT_TRUE(m_queue.TryDequeue(value, INFINITE));
                }
                ++m_itemsProcessed;
                // No sleeps because we're trying to maximize the chance of threads
                // interfering with each other.
            }
        }


        unsigned __int64 ProducerConsumerThread::GetItemsProcessed() const
        {
            return m_itemsProcessed;
        }


        bool ProducerConsumerThread::IsProducer() const
        {
            return m_isProducer;
        }
    }
}
