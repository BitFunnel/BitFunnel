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

#include "Commands.h"
#include "Environment.h"
#include "TaskPool.h"


namespace BitFunnel
{
    Environment::Environment(size_t threadCount)
        // TODO: Don't like passing *this to TaskFactory.
        // What if TaskFactory calls back before Environment is fully initialized?
        : m_taskFactory(new TaskFactory(*this)),
          m_taskPool(new TaskPool(threadCount))
    {
        RegisterCommands();
    }


    TaskFactory & Environment::GetTaskFactory() const
    {
        return *m_taskFactory;
    }


    TaskPool & Environment::GetTaskPool() const
    {
        return *m_taskPool;
    }


    void Environment::RegisterCommands()
    {
        DelayedPrint::Register(*m_taskFactory);
        Exit::Register(*m_taskFactory);
        Help::Register(*m_taskFactory);
    }
}
