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
#include <thread>       // sleep_for, this_thread

#include "Commands.h"
#include "Environment.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // Exit
    //
    //*************************************************************************
    Exit::Exit(Environment & environment)
        : TaskBase(environment, 0, Type::Exit)
    {
    }


    void Exit::Register(TaskFactory & factory)
    {
        std::unique_ptr<Descriptor>
            descriptor(new Descriptor(
                "quit",
                "waits for all current tasks to complete then exits.",
                "quit\n"
                "  Waits for all current tasks to complete then exits.",
                Create));

        factory.Register(std::move(descriptor));
    }


    std::unique_ptr<ITask>
        Exit::Create(Environment & environment,
                     Id /*id*/,
                     std::vector<std::string> const & /*tokens*/)
    {
        // TODO: error checking
        return std::unique_ptr<ITask>(
            new Exit(environment));
    }


    void Exit::Execute()
    {
        // Don't expect the Exit command to execute.
    }


    //*************************************************************************
    //
    // DelayedPrint
    //
    //*************************************************************************
    DelayedPrint::DelayedPrint(Environment & environment, Id id, char const * message)
        : TaskBase(environment, id, Type::Asynchronous),
        m_sleepTime(5),
        m_message(message)
    {
    }


    void DelayedPrint::Register(TaskFactory & factory)
    {
        std::unique_ptr<Descriptor>
            descriptor(new Descriptor(
                "delay",
                "Prints a message after certain number of seconds",
                "delay <message>\n"
                "  Waits for 5 seconds then prints <message> to the console."
                ,
                Create));
        factory.Register(std::move(descriptor));
    }


    std::unique_ptr<ITask>
        DelayedPrint::Create(Environment & environment,
                             Id id,
                             std::vector<std::string> const & tokens)
    {
        // TODO: error checking
        return std::unique_ptr<ITask>(
            new DelayedPrint(environment, id, tokens[1].c_str()));
    }


    void DelayedPrint::Execute()
    {
        std::this_thread::sleep_for(std::chrono::seconds(m_sleepTime));
        std::cout << GetId() << ": " << m_message << std::endl;
    }


    //*************************************************************************
    //
    // Help
    //
    //*************************************************************************
    Help::Help(Environment & environment,
               Id id,
               std::vector<std::string> const & tokens)
        : TaskBase(environment, id, Type::Synchronous)
    {
        if (tokens.size() > 1)
        {
            m_command = tokens[1];
        }
    }


    void Help::Register(TaskFactory & factory)
    {
        std::unique_ptr<Descriptor>
            descriptor(new Descriptor(
                "help",
                "Displays a list of available commands.",
                "help [<command>]\n"
                "  Displays help on a specific command.\n"
                "  If no command is specified, help displays\n"
                "  a list of available commands.",
                Create));
        factory.Register(std::move(descriptor));
    }


    std::unique_ptr<ITask>
        Help::Create(Environment & environment,
                     Id id,
                     std::vector<std::string> const & tokens)
    {
        return std::unique_ptr<ITask>(
            new Help(environment, id, tokens));
    }


    void Help::Execute()
    {
        if (m_command.size() > 0)
        {
            GetEnvironment().GetTaskFactory().Help(std::cout, m_command.c_str());
        }
        else
        {
            GetEnvironment().GetTaskFactory().Help(std::cout, nullptr);
        }
    }
}
