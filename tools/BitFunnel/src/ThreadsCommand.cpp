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
#include <string>

#include "Environment.h"
#include "ThreadsCommand.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // CompilerCommand
    //
    //*************************************************************************
    ThreadsCommand::ThreadsCommand(Environment & environment,
                                   Id id,
                                   char const * parameters)
        : TaskBase(environment, id, Type::Synchronous)
    {
        auto token = TaskFactory::GetNextToken(parameters);
        m_threadCount = stoull(token);
    }


    void ThreadsCommand::Execute()
    {
        GetEnvironment().SetThreadCount(m_threadCount);
        std::cout
            << "Matcher now using "
            << m_threadCount
            << " thread"
            << ((m_threadCount == 1) ? "" : "s")
            << "."
            << std::endl
            << std::endl;
    }


    ICommand::Documentation ThreadsCommand::GetDocumentation()
    {
        return Documentation(
            "threads",
            "Set the number of threads for query processing.",
            "threads <count>\n"
            "  Set the number of threads for query processing."
        );
    }
}
