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


#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Utilities/Factories.h"
#include "Headers.h"


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
            m_threads.push_back(new Thread(*this));
        }
        m_threadManager = Factories::CreateThreadManager(m_threads);
    }


    bool TaskPool::TryEnqueue(std::unique_ptr<ITask> task)
    {
        return m_queue.TryEnqueue(std::move(task));
    }


    TaskPool::Thread::Thread(TaskPool& pool)
        : m_pool(pool)
    {
    }


    void TaskPool::Thread::EntryPoint()
    {
        std::cout << "Thread entered." << std::endl;
        std::unique_ptr<ITask> task;
        while (m_pool.m_queue.TryDequeue(task))
        {
            task->Execute();
        }
        std::cout << "Thread exited." << std::endl;
    }


    void TaskPool::Shutdown()
    {
        m_queue.Shutdown();
        m_threadManager->WaitForThreads();
    }


    //*************************************************************************
    //
    // TaskFactory
    //
    //*************************************************************************

    TaskFactory::TaskFactory()
      : m_nextId(0)
    {
    }


    void TaskFactory::Register(char const * name,
                               char const * /*documentation*/,
                               ITask::Create create)
    {
        std::string s(name);
        auto it = m_taskMap.find(s);
        if (it != m_taskMap.end())
        {
            RecoverableError error("TaskFactory::Register: duplicate task name.");
            throw error;
        }

        m_taskMap.insert(std::make_pair(s, create));
    }


    std::unique_ptr<ITask> TaskFactory::CreateTask(char const * line)
    {
        auto tokens = Tokenize(line);

        if (tokens.size() == 0)
        {
            RecoverableError error("Expected a command.");
            throw error;
        }
        std::string const & name = tokens[0];

        auto it = m_taskMap.find(name);
        if (it == m_taskMap.end())
        {
            std::stringstream s;
            s << "Unknown command \"" << name << "\"";
            RecoverableError error(s.str().c_str());
            throw error;
        }

        return (*it).second(m_nextId++, tokens);
    }


    static void SkipWhite(char const * & text)
    {
        while (isspace(*text))
        {
            ++text;
            if (*text == '\0')
            {
                break;
            }
        }
    }


    static void Consume(char const * & text, char expected)
    {
        if (*text == expected)
        {
            ++text;
        }
        else
        {
            RecoverableError error("Syntax error.");
            throw error;
        }
    }


    ITask::Id TaskFactory::GetNextTaskId() const
    {
        return m_nextId;
    }


    std::vector<std::string> TaskFactory::Tokenize(char const * text)
    {
        std::vector<std::string> tokens;

        while (*text != '\0')
        {
            SkipWhite(text);

            if (*text == '"')
            {
                // Quoted string literal
                std::string s;
                ++text;
                while (*text != '"')
                {
                    if (*text == '\0')
                    {
                        RecoverableError error("Expected closing \" in string literal.");
                        throw error;
                    }
                    s.push_back(*text);
                    ++text;
                }
                Consume(text, '"');
                tokens.push_back(s);
            }
            else if (*text != '\0')
            {
                // Non-quoted literal.
                std::string s;
                while (!isspace(*text) && *text != '\0')
                {
                    s.push_back(*text);
                    ++text;
                }
                tokens.push_back(s);
            }
        }

        return tokens;
    }

    //*************************************************************************
    //
    // TaskBase
    //
    //*************************************************************************
    TaskBase::TaskBase(Id id, Type type)
        : m_id(id),
          m_type(type)
    {
    }


    ITask::Type TaskBase::GetType() const
    {
        return m_type;
    }


    ITask::Id TaskBase::GetId() const
    {
        return m_id;
    }


    //*************************************************************************
    //
    // Exit
    //
    //*************************************************************************
    Exit::Exit()
        : TaskBase(0, Type::Exit)
    {
    }


    void Exit::Register(TaskFactory & factory)
    {
        factory.Register("quit", "message", Create);
    }


    std::unique_ptr<ITask>
        Exit::Create(Id /*id*/,
                     std::vector<std::string> const & /*tokens*/)
    {
        // TODO: error checking
        return std::unique_ptr<ITask>(new Exit());
    }


    void Exit::Execute()
    {
    }


    //*************************************************************************
    //
    // DelayedPrint
    //
    //*************************************************************************
    DelayedPrint::DelayedPrint(Id id, char const * message)
        : TaskBase(id, Type::Asynchronous),
          m_sleepTime(5),
          m_message(message)
    {
    }


    void DelayedPrint::Register(TaskFactory & factory)
    {
        factory.Register("delay", "<message>", Create);
    }


    std::unique_ptr<ITask>
        DelayedPrint::Create(Id id,
                             std::vector<std::string> const & tokens)
    {
        // TODO: error checking
        return std::unique_ptr<ITask>(new DelayedPrint(id, tokens[1].c_str()));
    }


    void DelayedPrint::Execute()
    {
        std::this_thread::sleep_for(std::chrono::seconds(m_sleepTime));
        std::cout << GetId() << ": " << m_message << std::endl;
    }
}
