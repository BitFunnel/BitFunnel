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


#include <assert.h> // Temporary, until we have LogAssert.
#include <stddef.h>

//#include "BitFunnel/Factories.h"
//#include "LoggerInterfaces/Logging.h"
#include "ThreadManager.h"


namespace BitFunnel
{
    //IThreadManager* Factories::CreateThreadManager(const std::vector<IThreadBase*>& threads)
    //{
    //    return new ThreadManager(threads);
    //}


    ThreadManager::ThreadManager(const std::vector<IThreadBase*>& threads)
        : m_threads(threads)
    {
        for (size_t i = 0 ; i < threads.size(); ++i)
        {

            /*
            // TODO: Consider _beginthreadex and _endthreadex instead of CreateThread. 
            // According to MSDN documentation for CreateThread,
            // a thread in an executable that calls the C run-time library (CRT)
            // should use the _beginthreadex and _endthreadex functions for 
            // thread management rather than CreateThread and ExitThread; this 
            // requires the use of the multi-threaded version of the CRT. If a 
            // thread created using CreateThread calls the CRT, the CRT may 
            // terminate the process in low-memory conditions.
            DWORD threadId;
            HANDLE handle = CreateThread(
                0,              // Security attributes
                0,              // Stack size
                (LPTHREAD_START_ROUTINE)ThreadEntryPoint,
                threads[i],
                0,              // Creation flags
                &threadId);

            // LogAssertB(andle != nullptr, "Error: failed to start thread %d\n.", i);
            // TODO: port logging over.
            */

            m_handles.push_back(std::thread(ThreadEntryPoint, threads[i]));
        }
    }

    ThreadManager::~ThreadManager()
    {
        // REVIEW: what should this do if threads are still running?
    }

    void ThreadManager::WaitForThreads()
    {
        // TODO: does this make any sense?
        for (auto& handle : m_handles)
        {
            handle.join();
        }
    }

    void ThreadManager::ThreadEntryPoint(void* data)
    {
        IThreadBase* thread = static_cast<IThreadBase*>(data);
        thread->EntryPoint();
    }
}
