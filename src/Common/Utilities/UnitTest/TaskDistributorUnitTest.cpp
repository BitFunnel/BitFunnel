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

#include <chrono>
#include <thread>
#include <ostream>
#include <vector>

#include "BitFunnel/Utilities/ITaskProcessor.h"
#include "ThreadsafeCounter.h"
#include "TaskDistributor.h"
#include "gtest/gtest.h"


namespace BitFunnel
{
    namespace TaskDistributorUnitTest
    {
        void RunTest1(unsigned taskCount, unsigned maxSleepInMS);

        class TaskProcessor : public ITaskProcessor, NonCopyable
        {
        public:
            TaskProcessor(std::vector<ThreadsafeCounter64>& tasks,
                int maxSleepInMS,
                ThreadsafeCounter64& activeThreadCount);

            void ProcessTask(size_t taskId);
            void Finished();

            void Print(std::ostream& out);

        private:
            ThreadsafeCounter64& m_activeThreadCount;
            int m_callCount;
            std::vector<std::chrono::milliseconds> m_randomWaits;
            std::vector<ThreadsafeCounter64>& m_tasks;
        };


        //*************************************************************************
        //
        // TaskDistributorUnitTest
        //
        //*************************************************************************
        TEST(TaskDistributorUnitTest, Comprehensive)
        {
            RunTest1(100000, 0);
            RunTest1(1000, 20);
        }


        void RunTest1(unsigned taskCount, unsigned maxSleepInMS)
        {
            const int threadCount = 10;

            // The tasks vector will count the number of times each task is processed.
            std::vector<ThreadsafeCounter64> tasks;
            for (unsigned i = 0; i < taskCount; ++i)
            {
                tasks.push_back(ThreadsafeCounter64(0));
            }

            // Create one processor for each thread.
            srand(12345);
            ThreadsafeCounter64 activeThreadCount(threadCount);
            std::vector<ITaskProcessor*> processors;
            for (int i = 0 ; i < threadCount; ++i)
            {
                processors.push_back(new TaskProcessor(tasks, maxSleepInMS, activeThreadCount));
            }

            TaskDistributor distributor(processors, taskCount);
            distributor.WaitForCompletion();

            // Verify that ITaskProcessor::Finished() was called one time for each thread.
            ASSERT_EQ(activeThreadCount.ThreadsafeGetValue(), 0u);

            // Verify results that each task was done exactly once.
            for (unsigned i = 0; i < taskCount; ++i)
            {
                ASSERT_EQ(tasks[i].ThreadsafeGetValue(), 1u);
            }

            // Cleanup processors vector.
            for (int i = 0 ; i < threadCount; ++i)
            {
                delete processors[i];
            }
        }


        //*************************************************************************
        //
        // TaskProcessor
        //
        //*************************************************************************
        TaskProcessor::TaskProcessor(std::vector<ThreadsafeCounter64>& tasks,
                                     int maxSleepInMS,
                                     ThreadsafeCounter64& activeThreadCount)
            : m_activeThreadCount(activeThreadCount),
              m_callCount(0),
              m_tasks(tasks)
        {
            if (maxSleepInMS > 0)
            {
                // Generate random waits in constructor to make test more reproducible
                // than one that calls rand() from multiple threads.
                for (int i = 0; i < 100; ++i)
                {
                    // TODO: rand() % foo is a really bad RNG.
                    m_randomWaits.
                        push_back(std::chrono::milliseconds
                                  (rand() % maxSleepInMS));
                }
            }
        }


        void TaskProcessor::ProcessTask(size_t taskId)
        {
            ++m_callCount;

            m_tasks[taskId].ThreadsafeIncrement();

            if (m_randomWaits.size() > 0)
            {
                std::this_thread::sleep_for(m_randomWaits[m_callCount % m_randomWaits.size()]);
            }
        }


        void TaskProcessor::Finished()
        {
            m_activeThreadCount.ThreadsafeDecrement();
        }


        void TaskProcessor::Print(std::ostream& out)
        {
            out << "call count = " << m_callCount << std::endl;
        }
    }
}
