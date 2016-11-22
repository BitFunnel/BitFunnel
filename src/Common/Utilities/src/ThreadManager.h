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

#include <atomic>                                   // std::atomic embedded.
#include <thread>                                   // std::thread embedded.
#include <vector>                                   // std::vector embedded.

#include "BitFunnel/Utilities/IThreadManager.h"     // Base class.
#include "BitFunnel/Utilities/Stopwatch.h"          // Stopwatch embedded.
#include "BitFunnel/NonCopyable.h"                  // Base class.


namespace BitFunnel
{
    class ThreadManager : public IThreadManager, NonCopyable
    {
    public:
        // Starts one thread for each IThreadBase* in threads. Returns true if succesful.
        ThreadManager(const std::vector<std::unique_ptr<IThreadBase>>& threads);

        ~ThreadManager();

        //
        // IThreadManager methods.
        //

        // Returns the time in seconds since the first thread started.
        virtual double GetTimeSinceFirstThread() const override;

        // Wait for all threads to finish.
        virtual void WaitForThreads() override;

    private:
        void RecordThreadStart();
        static void ThreadEntryPoint(void* data);

        std::atomic<bool> m_firstThreadStarted;
        Stopwatch m_stopwatch;

        class ThreadWrapper
        {
        public:
            ThreadWrapper(ThreadManager& threadManager, IThreadBase& thread)
              : m_threadManager(threadManager),
                m_thread(thread)
            {
            }

            void Go()
            {
                m_threadManager.RecordThreadStart();
                m_thread.EntryPoint();
            }

        private:
            ThreadManager & m_threadManager;
            IThreadBase& m_thread;
        };

        std::vector<ThreadWrapper> m_wrappers;
        std::vector<std::thread> m_threads;
    };
}
