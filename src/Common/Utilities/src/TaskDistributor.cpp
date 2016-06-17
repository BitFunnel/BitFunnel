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

#include "BitFunnel/Utilities/Factories.h"
#include "BitFunnel/Utilities/ITaskProcessor.h"
#include "TaskDistributor.h"
#include "TaskDistributorThread.h"
#include "ThreadManager.h"


namespace BitFunnel
{
    std::unique_ptr<ITaskDistributor>
    Factories::CreateTaskDistributor(std::vector<std::unique_ptr<ITaskProcessor>> const & processors,
                                     size_t taskCount)
    {
        return std::unique_ptr<ITaskDistributor>(new TaskDistributor(processors, taskCount));
    }


    TaskDistributor::TaskDistributor(std::vector<std::unique_ptr<ITaskProcessor>> const & processors,
                                     size_t taskCount)
        : m_processors(processors),
          m_taskCount(taskCount),
          m_nextTaskId(0)
    {
        for (size_t i = 0 ; i < m_processors.size(); ++i)
        {
            m_threads.push_back(new TaskDistributorThread(*this, *m_processors[i]));
        }
        m_threadManager = new ThreadManager(m_threads);
    }


    TaskDistributor::~TaskDistributor()
    {
        for (size_t i = 0 ; i < m_threads.size(); ++i)
        {
            delete m_threads[i];
        }
        delete m_threadManager;
    }


    bool TaskDistributor::TryAllocateTask(size_t& taskId)
    {
        std::lock_guard<std::mutex> lock(m_lock);

        if (m_nextTaskId < m_taskCount)
        {
            taskId = m_nextTaskId++;
            return true;
        }
        else
        {
            return false;
        }
    }


    void TaskDistributor::WaitForCompletion()
    {
        m_threadManager->WaitForThreads();
    }
}
