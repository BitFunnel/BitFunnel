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
#include "CmdLineParser/CmdLineParser.h"
#include "Environment.h"
#include "REPL.h"
#include "TaskFactory.h"
#include "TaskPool.h"


namespace BitFunnel
{
    REPL::REPL(IFileSystem& /*fileSystem*/)
//      : m_fileSystem(fileSystem)
    {
    }


    int REPL::Main(std::istream& input,
                   std::ostream& output,
                   int argc,
                   char** argv)
    {
        CmdLine::CmdLineParser parser(
            "StatisticsBuilder",
            "Ingest documents and compute statistics about them.");

        CmdLine::RequiredParameter<char const *> path(
            "path",
            "Path to a tmp directory. "
            "Something like /tmp/ or c:\\temp\\, depending on platform..");

        // TODO: This parameter should be unsigned, but it doesn't seem to work
        // with CmdLineParser.
        CmdLine::OptionalParameter<int> gramSize(
            "gramsize",
            "Set the maximum ngram size for phrases.",
            1u);

        // TODO: This parameter should be unsigned, but it doesn't seem to work
        // with CmdLineParser.
        CmdLine::OptionalParameter<int> threadCount(
            "threads",
            "Set the thread count for ingestion and query processing.",
            1u);

        parser.AddParameter(path);
        parser.AddParameter(gramSize);
        parser.AddParameter(threadCount);

        int returnCode = 1;

        if (parser.TryParse(output, argc, argv))
        {
            try
            {
                Go(input, output, path, gramSize, threadCount);
                returnCode = 0;
            }
            catch (RecoverableError e)
            {
                output << "Error: " << e.what() << std::endl;
            }
            catch (...)
            {
                // TODO: Do we really want to catch all exceptions here?
                // Seems we want to at least print out the error message for BitFunnel exceptions.

                output << "Unexpected error.";
            }
        }

        return returnCode;
    }


    void REPL::Advice(std::ostream& output) const
    {
        output
            << "Index failed to load." << std::endl
            << std::endl
            << "Verify that directory path is valid and that the folder contains index files." << std::endl
            << "You can generate new index files with" << std::endl
            << "  BitFunnel statistics <manifest> <directory> -statistics" << std::endl
            << "  BitFunnel termtabe <directory>" << std::endl
            << "For more information run \"BitFunnel statistics -help\" and" << std::endl
            << "\"BitFunnel termtable -help\"." << std::endl;
    }


    void REPL::Go(std::istream& input,
                  std::ostream& output,
                  char const * directory,
                  size_t gramSize,
                  size_t threadCount) const
    {
        output
            << "Welcome to BitFunnel!" << std::endl
            << "Starting " << threadCount
            << " thread" << ((threadCount == 1) ? "" : "s") << std::endl
            << "(plus one extra thread for the Recycler." << std::endl
            << std::endl
            << "directory = \"" << directory << "\"" << std::endl
            << "gram size = " << gramSize << std::endl
            << std::endl;

        Environment environment(directory,
                                gramSize,
                                threadCount);

        output
            << "Starting index ..."
            << std::endl;

        try
        {
            environment.StartIndex();
        }
        catch (...)
        {
            Advice(output);
            throw;
        }

        output
            << "Index started successfully."
            << std::endl;

        output
            << std::endl
            << "Type \"help\" to get started." << std::endl
            << std::endl;

        TaskFactory & factory = environment.GetTaskFactory();
        TaskPool & taskPool = environment.GetTaskPool();

        for (;;)
        {
            try
            {
                output << factory.GetNextTaskId() << ": ";
                output.flush();

                std::string line;
                std::getline(input, line);

                std::unique_ptr<ICommand> task(factory.CreateTask(line.c_str()));

                if (task->GetType() == ICommand::Type::Exit)
                {
                    task->Execute();
                    break;
                }
                else if (task->GetType() == ICommand::Type::Asynchronous)
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
                output << "Error: " << e.what() << std::endl;
            }
            catch (...)
            {
                output << "Unknown error." << std::endl;
                throw;
            }
        }
        taskPool.Shutdown();
    }
}
