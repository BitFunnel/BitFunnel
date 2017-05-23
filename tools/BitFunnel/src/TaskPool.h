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

#include <memory>                               // std::unique_ptr embedded.
#include <vector>                               // std::vector embedded.

#include "BitFunnel/Utilities/BlockingQueue.h"  // BlockingQueue embedded.
#include "BitFunnel/Utilities/IThreadManager.h" // IThreadBase base class.


namespace BitFunnel
{
    class ITask;

    class TaskPool
    {
    public:
        TaskPool(size_t threadCount);
        ~TaskPool();

        bool TryEnqueue(std::unique_ptr<ITask> task);

        void Shutdown();

    private:
        class Thread : public IThreadBase
        {
        public:
            Thread(TaskPool& pool, size_t id);

            virtual void EntryPoint() override;

        private:
            TaskPool& m_pool;
            size_t m_id;
        };

        // TODO: Convert ThreadManager to use std::vector<std::unique_ptr<IThreadBase>>
        std::vector<std::unique_ptr<IThreadBase>> m_threads;
        std::unique_ptr<IThreadManager> m_threadManager;

        BlockingQueue<std::unique_ptr<ITask>> m_queue;

        bool m_shutdownInitiated;
    };
}
