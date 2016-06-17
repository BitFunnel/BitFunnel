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

#include <array>
#include <atomic>
#include <chrono>
#include <thread>
#include <ostream>
#include <vector>

#include "BitFunnel/Utilities/Factories.h"
#include "BitFunnel/Utilities/ITaskProcessor.h"
#include "TaskDistributor.h"
#include "gtest/gtest.h"


namespace BitFunnel
{
    namespace TaskDistributorUnitTest
    {

        // Hard NUM_TASKSis a hack. This was done when ripping out
        // ThreadsafeCounter in favor of std::atomic because this resulted
        // in the quickest possible change.
        #define NUM_TASKS 100

        void RunTest1(unsigned taskCount, unsigned maxSleepInMS);

        class TaskProcessor : public ITaskProcessor, NonCopyable
        {
        public:
            TaskProcessor(std::array<std::atomic<uint64_t>, NUM_TASKS>& tasks,
                int maxSleepInMS,
                std::atomic<uint64_t>& activeThreadCount);

            void ProcessTask(size_t taskId);
            void Finished();

            void Print(std::ostream& out);

        private:
            std::atomic<uint64_t>& m_activeThreadCount;
            int m_callCount;
            std::vector<std::chrono::milliseconds> m_randomWaits;
            std::array<std::atomic<uint64_t>, NUM_TASKS>& m_tasks;
        };


        //*************************************************************************
        //
        // TaskDistributorUnitTest
        //
        //*************************************************************************
        TEST(TaskDistributorUnitTest, Comprehensive)
        {
            RunTest1(NUM_TASKS, 0);
            RunTest1(NUM_TASKS, 3);
        }


        void RunTest1(unsigned taskCount, unsigned maxSleepInMS)
        {
            const int threadCount = 10;

            // The tasks vector will count the number of times each task is processed.
            std::array<std::atomic<uint64_t>, NUM_TASKS> tasks = {};

            // Create one processor for each thread.
            // TODO: Review use of srand in BitFunnel and unit tests.
            // Usually we use a different random number generator for a variety of reasons.
            srand(12345);
            std::atomic<uint64_t> activeThreadCount(threadCount);
            std::vector<std::unique_ptr<ITaskProcessor>> processors;
            for (int i = 0 ; i < threadCount; ++i)
            {
                processors.push_back(std::unique_ptr<ITaskProcessor>(new TaskProcessor(tasks, maxSleepInMS, activeThreadCount)));
            }

            std::unique_ptr<ITaskDistributor> distributor(Factories::CreateTaskDistributor(processors, taskCount));
            distributor->WaitForCompletion();

            // Verify that ITaskProcessor::Finished() was called one time for each thread.
            ASSERT_EQ(activeThreadCount.load(), 0u);

            // Verify results that each task was done exactly once.
            for (unsigned i = 0; i < taskCount; ++i)
            {
                ASSERT_EQ(tasks[i].load(), 1u);
            }
        }


        //*************************************************************************
        //
        // TaskProcessor
        //
        //*************************************************************************
        TaskProcessor::TaskProcessor(std::array<std::atomic<uint64_t>, NUM_TASKS>& tasks,
                                     int maxSleepInMS,
                                     std::atomic<uint64_t>& activeThreadCount)
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

            ++m_tasks[taskId];

            if (m_randomWaits.size() > 0)
            {
                std::this_thread::sleep_for(m_randomWaits[m_callCount % m_randomWaits.size()]);
            }
        }


        void TaskProcessor::Finished()
        {
            --m_activeThreadCount;
        }


        void TaskProcessor::Print(std::ostream& out)
        {
            out << "call count = " << m_callCount << std::endl;
        }
    }
}
