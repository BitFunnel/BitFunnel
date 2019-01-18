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
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Utilities/ReadLines.h"
#include "CmdLineParser/CmdLineParser.h"
#include "Environment.h"
#include "LoggerInterfaces/Check.h"
#include "REPL.h"


namespace BitFunnel
{
    REPL::REPL(IFileSystem& fileSystem)
      : m_fileSystem(fileSystem)
    {
    }


    int REPL::Main(std::istream& input,
                   std::ostream& output,
                   int argc,
                   char const *argv[])
    {
        CmdLine::CmdLineParser parser(
            "Repl",
            "Interactively process user commands.");

        CmdLine::RequiredParameter<char const *> path(
            "config",
            "Path to a configuration directory containing files produced by "
            "the 'BitFunnel statistics' and 'BitFunnel termtable' commands.");

        // TODO: This parameter should be unsigned, but it doesn't seem to work
        // with CmdLineParser.
        CmdLine::OptionalParameter<int> gramSize(
            "gramsize",
            "Set the maximum ngram size for phrases.",
            1u,
            CmdLine::GreaterThan(0));

        // TODO: This parameter should be unsigned, but it doesn't seem to work
        // with CmdLineParser.
        CmdLine::OptionalParameter<int> threadCount(
            "threads",
            "Set the thread count for ingestion and query processing.",
            1u,
            CmdLine::GreaterThan(0));

        // TODO: This parameter should be unsigned, but it doesn't seem to work
        // with CmdLineParser.
        CmdLine::OptionalParameter<int> memory(
            "memory",
            "Specify the amount of memory (in KiB) to use for Slice buffers.",
            1000000u,
            CmdLine::GreaterThan(0));

        CmdLine::OptionalParameter<char const *> scriptFile(
            "script",
            "File with commands to execute.",
            nullptr);

        // TODO: This parameter should be unsigned, but it doesn't seem to work
        // with CmdLineParser.
        CmdLine::OptionalParameter<int> restore(
            "restore",
            "Specify non-zero number to re-load saved slices.",
            0u,
            CmdLine::GreaterThan(0));

        parser.AddParameter(path);
        parser.AddParameter(gramSize);
        parser.AddParameter(threadCount);
        parser.AddParameter(memory);
        parser.AddParameter(scriptFile);
        parser.AddParameter(restore);

        int returnCode = 1;

        if (parser.TryParse(output, argc, argv))
        {
            try
            {
                // TODO: these casts can be removed when gramSize and
                // threadCount are fixed to be unsigned.
                Go(input,
                   output,
                   path,
                   static_cast<size_t>(gramSize),
                   static_cast<size_t>(threadCount),
                   static_cast<size_t>(memory) * 1024ull,
                   static_cast<size_t>(restore),
                   scriptFile);
                returnCode = 0;
            }
            catch (RecoverableError e)
            {
                output << "Error (RecoverableError): " << e.what() << std::endl;
                Advice(output);
            }
            catch (FatalError e)
            {
                output << "Error (FatalError): " << e.what() << std::endl;
                Advice(output);
            }
            catch (Logging::CheckException e)
            {
                output
                    << "Error (CheckException): "
                    << e.GetMessage()
                    << std::endl;
                Advice(output);
            }
            catch (std::runtime_error e)
            {
                output << "Runtime exception: " << e.what() << std::endl;
                Advice(output);
            }
            catch (...)
            {
                output << "Unknown error." << std::endl;
                Advice(output);
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
            << "  BitFunnel statistics <manifest> <directory>" << std::endl
            << "  BitFunnel termtable <directory> <density> <treatment>" << std::endl
            << "For more information run \"BitFunnel statistics -help\" and" << std::endl
            << "\"BitFunnel termtable -help\"." << std::endl;
    }


    void REPL::Go(std::istream& input,
                  std::ostream& output,
                  char const * directory,
                  size_t gramSize,
                  size_t threadCount,
                  size_t memory,
                  size_t restore,
                  char const * scriptFile) const
    {
        output
            << "Welcome to BitFunnel!" << std::endl
            << "Starting " << threadCount
            << " thread" << ((threadCount == 1) ? "" : "s") << std::endl
            << "(plus one extra thread for the Recycler.)" << std::endl
            << std::endl
            << "directory = \"" << directory << "\"" << std::endl
            << "gram size = " << gramSize << std::endl
            << std::endl;

        Environment environment(m_fileSystem,
                                output,
                                directory,
                                gramSize,
                                threadCount,
                                memory);

        output
            << "Starting index ..."
            << std::endl;

        environment.StartIndex();
        environment.SetShards(0, environment.GetIngestor().GetShardCount() - 1);
        if (restore)
        {
            auto & fileManager = environment.GetSimpleIndex().GetFileManager();
            environment.GetIngestor().TemporaryReadAllSlices(fileManager);
        }

        Loop(environment,
                input,
                output,
                scriptFile);
    }


    void REPL::Loop(Environment& environment,
                    std::istream& input,
                    std::ostream& output,
                    char const * scriptFile) const
    {
        output
            << "Index started successfully."
            << std::endl;

        output
            << std::endl
            << "Type \"help\" to get started." << std::endl
            << std::endl;

        TaskFactory & factory = environment.GetTaskFactory();
        TaskPool & taskPool = environment.GetTaskPool();


        std::vector<std::string> lines;
        if (scriptFile != nullptr)
        {
            // Load commands into vector and then execute commands.
            lines = ReadLines(m_fileSystem, scriptFile);
        }

        auto it = lines.begin();

        for (;;)
        {
            try
            {
                output << factory.GetNextTaskId() << ": ";
                output.flush();

                // Handle case for script file vs. console.
                std::string line;
                if (it != lines.end())
                {
                    // Input coming from script file.
                    line = *it++;
                    output << line << std::endl;
                }
                else
                {
                    // Input coming from console.
                    // Check for eof to handle the case where input is not cin.
                    if (input.eof())
                    {
                        break;
                    }

                    std::getline(input, line);
                }

                if (line.length() != 0)
                {
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
            }
            catch (RecoverableError e)
            {
                output << "Error (RecoverableError): " << e.what() << std::endl;
                if (environment.GetFailOnException())
                {
                    throw e;
                }
            }
            catch (Logging::CheckException e)
            {
                // TODO: We probably shouldn't catch CheckException here since
                // we don't know whether the situation is safely recoverable.
                // https://github.com/BitFunnel/BitFunnel/issues/425
                output << "Error (CheckException): " << e.GetMessage() << std::endl;
                if (environment.GetFailOnException())
                {
                    throw e;
                }
            }
        }
        taskPool.Shutdown();
    }
}
