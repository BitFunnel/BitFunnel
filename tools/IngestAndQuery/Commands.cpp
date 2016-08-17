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

#include "BitFunnel/Exceptions.h"
#include "Commands.h"
#include "Environment.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // DelayedPrint
    //
    //*************************************************************************
    DelayedPrint::DelayedPrint(Environment & environment,
                               Id id,
                               char const * parameters)
        : TaskBase(environment, id, Type::Asynchronous),
        m_sleepTime(5)
    {
        m_message = parameters;
    }


    void DelayedPrint::Execute()
    {
        std::this_thread::sleep_for(std::chrono::seconds(m_sleepTime));
        std::cout << GetId() << ": " << m_message << std::endl;
    }


    ITask::Documentation DelayedPrint::GetDocumentation()
    {
        return Documentation(
            "delay",
            "Prints a message after certain number of seconds",
            "delay <message>\n"
            "  Waits for 5 seconds then prints <message> to the console."
            );
    }


    //*************************************************************************
    //
    // Exit
    //
    //*************************************************************************
    Exit::Exit(Environment & environment,
               Id id,
               char const * /*parameters*/)
        : TaskBase(environment, id, Type::Exit)
    {
    }


    void Exit::Execute()
    {
        std::cout
            << std::endl
            << "Initiating shutdown sequence ..."
            << std::endl;
    }


    ITask::Documentation Exit::GetDocumentation()
    {
        return Documentation(
            "quit",
            "waits for all current tasks to complete then exits.",
            "quit\n"
            "  Waits for all current tasks to complete then exits."
            );
    }


    //*************************************************************************
    //
    // Help
    //
    //*************************************************************************
    Help::Help(Environment & environment,
               Id id,
               char const * parameters)
        : TaskBase(environment, id, Type::Synchronous)
    {
        m_command = TaskFactory::GetNextToken(parameters);
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


    ITask::Documentation Help::GetDocumentation()
    {
        return Documentation(
            "help",
            "Displays a list of available commands.",
            "help [<command>]\n"
            "  Displays help on a specific command.\n"
            "  If no command is specified, help displays\n"
            "  a list of available commands."
            );
    }


    //*************************************************************************
    //
    // Ingest
    //
    //*************************************************************************
    Ingest::Ingest(Environment & environment,
                   Id id,
                   char const * parameters)
        : TaskBase(environment, id, Type::Synchronous)
    {
        auto command = TaskFactory::GetNextToken(parameters);
        if (command.compare("manifest") == 0)
        {
            m_manifest = true;
        }
        else
        {
            m_manifest = false;
            if (command.compare("chunk") != 0)
            {
                RecoverableError error("Ingest expects \"chunk\" or \"manifest\".");
                throw error;
            }
        }

        m_path = TaskFactory::GetNextToken(parameters);
    }


    void Ingest::Execute()
    {
        std::cout
            << "Ingesting "
            << (m_manifest ? "manifest " : "chunk ")
            << "\"" << m_path << "\""
            << std::endl;
    }


    ITask::Documentation Ingest::GetDocumentation()
    {
        return Documentation(
            "ingest",
            "Ingests documents into the index.",
            "ingest (manifest | chunk) <path>\n"
            "  Ingests a single chunk file or a list of chunk\n"
            "  files specified by a manifest."
            );
    }


    //*************************************************************************
    //
    // Query
    //
    //*************************************************************************
    Query::Query(Environment & environment,
                 Id id,
                 char const * parameters)
        : TaskBase(environment, id, Type::Synchronous)
    {
        auto command = TaskFactory::GetNextToken(parameters);
        if (command.compare("one") == 0)
        {
            m_isSingleQuery = true;
            m_query = parameters;
        }
        else
        {
            m_isSingleQuery = false;
            if (command.compare("log") != 0)
            {
                RecoverableError error("Query expects \"one\" or \"log\".");
                throw error;
            }
            m_query = TaskFactory::GetNextToken(parameters);
        }
    }


    void Query::Execute()
    {
        if (m_isSingleQuery)
        {
            std::cout
                << "Processing query \""
                << m_query
                << "\"" << std::endl;
        }
        else
        {
            std::cout
                << "Processing queries from log at \""
                << m_query
                << "\"" << std::endl;
        }
    }


    ITask::Documentation Query::GetDocumentation()
    {
        return Documentation(
            "query",
            "Process a single query or list of queries.",
            "query (one <expression>) | (log <file>)\n"
            "  Processes a single query or a list of queries\n"
            "  specified by a file."
            );
    }


    //*************************************************************************
    //
    // Status
    //
    //*************************************************************************
    Status::Status(Environment & environment,
                   Id id,
                   char const * /*parameters*/)
        : TaskBase(environment, id, Type::Synchronous)
    {
    }


    void Status::Execute()
    {
        std::cout
            << "Printing system status ..."
            << std::endl;
    }


    ITask::Documentation Status::GetDocumentation()
    {
        return Documentation(
            "status",
            "Prints system status.",
            "status\n"
            "  Prints system status."
            );
    }
}
