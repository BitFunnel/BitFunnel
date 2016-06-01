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
#include "LoggerInterfaces/Logging.h"

namespace BitFunnel
{
    BlockingQueueBase::BlockingQueueBase(unsigned capacity)
        : m_shutdownEvent(CreateEvent(nullptr, TRUE, FALSE, nullptr)),
          m_enqueueSemaphore(CreateSemaphore(nullptr, capacity, capacity, nullptr)),
          m_dequeueSemaphore(CreateSemaphore(nullptr, 0, capacity, nullptr))
    {
        LogAssertB(m_shutdownEvent != nullptr);
        LogAssertB(m_enqueueSemaphore != nullptr);
        LogAssertB(m_dequeueSemaphore != nullptr);
    }


    BlockingQueueBase::~BlockingQueueBase()
    {
        Shutdown();

        CloseHandle(m_dequeueSemaphore);
        CloseHandle(m_enqueueSemaphore);
        CloseHandle(m_shutdownEvent);
    }


    void BlockingQueueBase::Shutdown()
    {
        SetEvent(m_shutdownEvent);
    }


    bool BlockingQueueBase::TryDequeue(unsigned timeoutInMS)
    {
        HANDLE handles[2] = {m_shutdownEvent, m_dequeueSemaphore};

        return WaitForMultipleObjects(2, handles, FALSE, timeoutInMS) == 1;
    }


    bool BlockingQueueBase::TryEnqueue(unsigned timeoutInMS)
    {
        HANDLE handles[2] = {m_shutdownEvent, m_enqueueSemaphore};

        return WaitForMultipleObjects(2, handles, FALSE, timeoutInMS) == 1;
    }


    void BlockingQueueBase::CompleteDequeue()
    {
        ReleaseSemaphore(m_enqueueSemaphore, 1, nullptr);
    }


    void BlockingQueueBase::CompleteEnqueue()
    {
        ReleaseSemaphore(m_dequeueSemaphore, 1, nullptr);
    }


    bool BlockingQueueBase::IsShuttingDown() const
    {
        return WaitForSingleObject(m_shutdownEvent, 0) == WAIT_OBJECT_0;
    }
}
