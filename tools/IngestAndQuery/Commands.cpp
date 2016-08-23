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
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/IngestChunks.h"
#include "BitFunnel/ITermTable2.h"
#include "BitFunnel/RowIdSequence.h"
#include "BitFunnel/Term.h"
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


    ICommand::Documentation DelayedPrint::GetDocumentation()
    {
        return Documentation(
            "delay",
            "Prints a message after certain number of seconds",
            "delay <message>\n"
            "  Sample command to test multi-threading architecture.\n"
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


    ICommand::Documentation Exit::GetDocumentation()
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


    ICommand::Documentation Help::GetDocumentation()
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
        else if (command.compare("chunk") == 0)
        {
            m_manifest = false;
        }
        else
        {
            RecoverableError error("Ingest expects \"chunk\" or \"manifest\".");
            throw error;
        }

        m_path = TaskFactory::GetNextToken(parameters);
    }


    void Ingest::Execute()
    {
        if (m_manifest)
        {
            std::cout << "Ingest manifest not implemented." << std::endl;
        }
        else
        {
            std::vector<std::string> filePaths;
            filePaths.push_back(m_path);
            std::cout
                << "Ingesting chunk file \""
                << filePaths.back()
                << "\"" << std::endl;

            Environment & environment = GetEnvironment();
            IConfiguration const & configuration = environment.GetConfiguration();
            IIngestor & ingestor = environment.GetIngestor();
            size_t threadCount = 1;

            IngestChunks(filePaths, configuration, ingestor, threadCount);

            std::cout << "Ingestion complete." << std::endl;
        }
    }


    ICommand::Documentation Ingest::GetDocumentation()
    {
        return Documentation(
            "ingest",
            "Ingests documents into the index. (TODO)",
            "ingest (manifest | chunk) <path>\n"
            "  Ingests a single chunk file or a list of chunk\n"
            "  files specified by a manifest.\n"
            "  NOT IMPLEMENTED"
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
        std::cout << "NOT IMPLEMENTED" << std::endl;
    }


    ICommand::Documentation Query::GetDocumentation()
    {
        return Documentation(
            "query",
            "Process a single query or list of queries. (TODO)",
            "query (one <expression>) | (log <file>)\n"
            "  Processes a single query or a list of queries\n"
            "  specified by a file.\n"
            "  NOT IMPLEMENTED"
            );
    }


    //*************************************************************************
    //
    // Script
    //
    //*************************************************************************
    Script::Script(Environment & environment,
                   Id id,
                   char const * /*parameters*/)
        : TaskBase(environment, id, Type::Synchronous)
    {
    }


    void Script::Execute()
    {
        std::cout
            << "Running script ..." << std::endl
            << "NOT IMPLEMENTED" << std::endl
            << std::endl;
    }


    ICommand::Documentation Script::GetDocumentation()
    {
        return Documentation(
            "script",
            "Runs commands from a file.(TODO)",
            "script <filename>\n"
            "  Runs commands from a file.\n"
            "  NOT IMPLEMENTED"
            );
    }


    //*************************************************************************
    //
    // Show
    //
    //*************************************************************************
    Show::Show(Environment & environment,
               Id id,
               char const * parameters)
        : TaskBase(environment, id, Type::Synchronous)
    {
        auto command = TaskFactory::GetNextToken(parameters);
        if (command.compare("term") == 0)
        {
            m_mode = Mode::Term;
            m_term = TaskFactory::GetNextToken(parameters);
        }
        else if (command.compare("rows") == 0)
        {
            m_mode = Mode::Rows;
            m_term = TaskFactory::GetNextToken(parameters);
        }
        else
        {
            RecoverableError error("Show expects \"term\" or \"rows\" (for now).");
            throw error;
        }
    }


    void Show::Execute()
    {
        // TODO: Consider parsing phrase terms here.

        auto & environment = GetEnvironment();
        Term term(m_term.c_str(), 0, environment.GetConfiguration());
        RowIdSequence rows(term, environment.GetTermTable());

        std::cout
            << "Term("
            << "\"" << m_term << "\""
            << ")" << std::endl;

        for (auto row : rows)
        {
            std::cout
                << "  RowId("
                << row.GetRank()
                << ", "
                << row.GetIndex()
                << ")";

            if (m_mode == Mode::Rows)
            {
                IIngestor & ingestor = GetEnvironment().GetIngestor();

                // TODO: Figure out how to supply the DocId. The DocId is used
                // to gain access to a Slice.
                // For now use the DocId of the first document in
                // Wikipedia chunk AA\wiki_00.
                const DocId docId = 12;
                auto handle = ingestor.GetHandle(docId);
                std::cout << ": " << (handle.GetBit(row) ? "1" : "0");
            }

            std::cout << std::endl;
        }
    }


    ICommand::Documentation Show::GetDocumentation()
    {
        return Documentation(
            "show",
            "Shows information about various data structures. (TODO)",
            "show (rows <term> [<docstart> <docend>])\n"
            "   | (term <term>)\n"
            "   | shards\n"
            "   | shard <shardid>\n"
            "  Shows information about various data structures."
            "  NOT IMPLEMENTED\n"
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
            << std::endl
            << "NOT IMPLEMENTED"
            << std::endl;

        double bytesPerDocument = 0;
        for (Rank rank = 0; rank < c_maxRankValue; ++rank)
        {
            bytesPerDocument += GetEnvironment().GetTermTable().GetBytesPerDocument(rank);
            std::cout
                << rank
                << ": "
                << GetEnvironment().GetTermTable().GetBytesPerDocument(rank)
                << " bytes/document"
                << std::endl;
        }

        std::cout
            << "Total: "
            << bytesPerDocument
            << " bytes/document"
            << std::endl;

        std::cout << std::endl;

        for (Rank rank = 0; rank < c_maxRankValue; ++rank)
        {
            std::cout
                << rank
                << ": "
                << GetEnvironment().GetTermTable().GetTotalRowCount(rank)
                << " rows."
                << std::endl;
        }
    }


    ICommand::Documentation Status::GetDocumentation()
    {
        return Documentation(
            "status",
            "Prints system status.",
            "status\n"
            "  Prints system status."
            "  NOT IMPLEMENTED\n"
            );
    }
}
