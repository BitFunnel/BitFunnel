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

#include <iostream>

#include "BitFunnel/Exceptions.h"
#include "Environment.h"
#include "TaskFactory.h"
#include "TaskPool.h"
#include "REPL.h"


namespace BitFunnel
{
    void REPL(char const * directory,
              size_t gramSize,
              size_t threadCount)
    {
        std::cout
            << "Welcome to BitFunnel!" << std::endl
            << "Starting " << threadCount
            << " thread" << ((threadCount == 1) ? "" : "s") << std::endl
            << std::endl
            << "directory = \"" << directory << "\"" << std::endl
            << "gram size = " << gramSize << std::endl
            << std::endl;

        std::cout
            << std::endl
            << "Type \"help\" to get started." << std::endl
            << std::endl;

        Environment environment(directory,
                                gramSize,
                                threadCount);
        TaskFactory & factory = environment.GetTaskFactory();
        TaskPool & taskPool = environment.GetTaskPool();

        for (;;)
        {
            try
            {
                std::cout << factory.GetNextTaskId() << ": ";
                std::cout.flush();

                std::string line;
                std::getline(std::cin, line);

                std::unique_ptr<ITask> task(factory.CreateTask(line.c_str()));

                if (task->GetType() == ITask::Type::Exit)
                {
                    task->Execute();
                    break;
                }
                else if (task->GetType() == ITask::Type::Asynchronous)
                {
                    taskPool.TryEnqueue(std::move(task));
                }
                else
                {
                    task->Execute();
                }
            }
            catch (RecoverableError e)
            {
                std::cout
                    << "Error: "
                    << e.what()
                    << std::endl;
            }
        }

        taskPool.Shutdown();
    }
}
