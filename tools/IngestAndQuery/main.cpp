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

#include "BitFunnel/Exceptions.h"
#include "Environment.h"
#include "TaskFactory.h"
#include "TaskPool.h"


namespace BitFunnel
{
    void Test()
    {
        const size_t threadCount = 1;

        Environment environment(threadCount);
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


int main(int /*argc*/, char** /*argv*/)
{
    BitFunnel::Test();
}
