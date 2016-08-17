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

#include <iostream>

#include "BitFunnel/Utilities/Factories.h"
#include "TaskFactory.h"
#include "TaskPool.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // TaskPool
    //
    //*************************************************************************
    TaskPool::TaskPool(size_t threadCount)
      : m_queue(100)
    {
        for (size_t i = 0; i < threadCount; ++i)
        {
            m_threads.push_back(new Thread(*this, i));
        }
        m_threadManager = Factories::CreateThreadManager(m_threads);
    }


    bool TaskPool::TryEnqueue(std::unique_ptr<ICommand> task)
    {
        return m_queue.TryEnqueue(std::move(task));
    }


    TaskPool::Thread::Thread(TaskPool& pool, size_t id)
      : m_pool(pool),
        m_id(id)
    {
    }


    void TaskPool::Thread::EntryPoint()
    {
        std::unique_ptr<ICommand> task;
        while (m_pool.m_queue.TryDequeue(task))
        {
            task->Execute();
        }
        std::cout << "Thread " << m_id << " exited." << std::endl;
    }


    void TaskPool::Shutdown()
    {
        m_queue.Shutdown();
        m_threadManager->WaitForThreads();
    }
}
