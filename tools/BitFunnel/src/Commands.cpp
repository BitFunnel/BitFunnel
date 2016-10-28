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
#include <iomanip>
#include <iostream>
#include <istream>
#include <string>
#include <thread>           // sleep_for, this_thread
#include <unordered_map>    // For debugging.

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/Configuration/IStreamConfiguration.h"
#include "BitFunnel/Data/Sonnets.h"
#include "BitFunnel/Exceptions.h"
#include "BitFunnel/IDiagnosticStream.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IChunkManifestIngestor.h"
#include "BitFunnel/Index/IDocument.h"
#include "BitFunnel/Index/IDocumentCache.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/IngestChunks.h"
#include "BitFunnel/Index/ITermTable.h"
#include "BitFunnel/Index/RowIdSequence.h"
#include "BitFunnel/Plan/Factories.h"
#include "BitFunnel/Plan/IMatchVerifier.h"
#include "BitFunnel/Plan/QueryInstrumentation.h"
#include "BitFunnel/Plan/QueryPipeline.h"
#include "BitFunnel/Plan/QueryRunner.h"
#include "BitFunnel/Plan/TermMatchTreeEvaluator.h"
#include "BitFunnel/Term.h"
#include "BitFunnel/Utilities/Factories.h"
#include "BitFunnel/Utilities/ReadLines.h"
#include "Commands.h"
#include "CsvTsv/Csv.h"
#include "CsvTsv/Table.h"
#include "Environment.h"
#include "LoggerInterfaces/Check.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // Analyze - analyze row table densities.
    //
    //*************************************************************************
    Analyze::Analyze(Environment & environment,
                     Id id,
                     char const * /*parameters*/)
        : TaskBase(environment, id, Type::Synchronous)
    {
    }


    void Analyze::Execute()
    {
        CHECK_NE(*GetEnvironment().GetOutputDir().c_str(), '\0')
            << "Output directory not set. "
            << "Please use the 'cd' command to set an "
            << "output directory";

        Factories:: AnalyzeRowTables(GetEnvironment().GetSimpleIndex(),
                                     GetEnvironment().GetOutputDir().c_str());
    }


    ICommand::Documentation Analyze::GetDocumentation()
    {
        return Documentation(
            "analyze",
            "Analyzes RowTables statistics (e.g. row and column densities).",
            "analyze\n"
            "  Analyzes RowTables statistics (e.g. row and column densities).\n"
        );
    }


    //*************************************************************************
    //
    // Cd - change output directory
    //
    //*************************************************************************
    Cd::Cd(Environment & environment,
           Id id,
           char const * parameters)
        : TaskBase(environment, id, Type::Synchronous)
    {
        m_dir = TaskFactory::GetNextToken(parameters);
    }


    void Cd::Execute()
    {
        GetEnvironment().SetOutputDir(m_dir);
        std::cout
            << "output directory is now \""
            << m_dir
            << "\"." << std::endl;
    }


    ICommand::Documentation Cd::GetDocumentation()
    {
        return Documentation(
            "cd",
            "Change the output directory",
            "cd <path>\n"
            "  Changes the output directory to the specified path.\n"
        );
    }


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
            "Waits for all current tasks to complete then exits.",
            "quit\n"
            "  Waits for all current tasks to complete then exits."
            );
    }


    //*************************************************************************
    //
    // FailOnException
    //
    //*************************************************************************
    FailOnException::FailOnException(Environment & environment,
               Id id,
               char const * /*parameters*/)
        : TaskBase(environment, id, Type::Synchronous)
    {
    }


    void FailOnException::Execute()
    {
        GetEnvironment().SetFailOnException(true);
    }


    ICommand::Documentation FailOnException::GetDocumentation()
    {
        return Documentation(
            "failOnException",
            "Forces application failure when exceptions are thrown.",
            "failOnException\n"
            "  Forces application failure when exceptions are thrown."
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
                   char const * parameters,
                   bool cacheDocuments)
        : TaskBase(environment, id, Type::Synchronous),
          m_cacheDocuments(cacheDocuments)
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
        if (m_manifest && m_path.compare("sonnets") == 0)
        {
                Environment & environment = GetEnvironment();
                IConfiguration const & configuration =
                    environment.GetConfiguration();
                IIngestor & ingestor = environment.GetIngestor();
                size_t threadCount = 1;

                auto manifest = Factories::CreateBuiltinChunkManifest(
                    Sonnets::chunks,
                    configuration,
                    ingestor,
                    m_cacheDocuments);

                IngestChunks(*manifest, threadCount);

                std::cout << "Ingestion complete." << std::endl;
        }
        else
        {
            std::vector<std::string> filePaths;

            if (m_manifest)
            {
                std::cout
                    << "Ingesting manifest \""
                    << m_path
                    << "\"" << std::endl;

                filePaths = ReadLines(GetEnvironment().GetFileSystem(),
                                      m_path.c_str());

            }
            else
            {
                filePaths.push_back(m_path);
                std::cout
                    << "Ingesting chunk file \""
                    << filePaths.back()
                    << "\"" << std::endl;
            }

            if (m_cacheDocuments)
            {
                std::cout
                    << "Caching IDocuments for query verification."
                    << std::endl;
            }

            Environment & environment = GetEnvironment();
            IFileSystem & fileSystem =
                environment.GetFileSystem();
            IConfiguration const & configuration =
                environment.GetConfiguration();
            IIngestor & ingestor = environment.GetIngestor();
            size_t threadCount = 1;

            auto manifest = Factories::CreateChunkManifestIngestor(
                fileSystem,
                filePaths,
                configuration,
                ingestor,
                m_cacheDocuments);

            IngestChunks(*manifest, threadCount);

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
    // Cache
    //
    //*************************************************************************
    Cache::Cache(Environment & environment,
                 Id id,
                 char const * parameters)
        : Ingest(environment, id, parameters, true)
    {
    }


    ICommand::Documentation Cache::GetDocumentation()
    {
        return Documentation(
            "cache",
            "Ingests documents into the index and also stores them in a cache\n"
            "for query verification purposes.",
            "cache (manifest | chunk) <path>\n"
            "  Ingests a single chunk file or a list of chunk\n"
            "  files specified by a manifest.\n"
            "  Also caches IDocuments for query verification.\n"
            );
    }


    //*************************************************************************
    //
    // Load
    //
    //*************************************************************************
    Load::Load(Environment & environment,
               Id id,
               char const * parameters)
        : Ingest(environment, id, parameters, false)
    {
    }


    ICommand::Documentation Load::GetDocumentation()
    {
        return Documentation(
            "load",
            "Ingests documents into the index",
            "load (manifest | chunk) <path>\n"
            "  Ingests a single chunk file or a list of chunk\n"
            "  files specified by a manifest.\n"
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
                std::cout << "expected log or one" << std::endl;
                throw RecoverableError();
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
            std::cout << "NOT IMPLEMENTED" << std::endl;
        }
        else
        {
            CHECK_NE(*GetEnvironment().GetOutputDir().c_str(), '\0')
                << "Output directory not set. "
                << "Please use the 'cd' command to set an "
                << "output directory";

            std::cout
                << "Processing queries from log at \""
                << m_query
                << "\"" << std::endl;

            std::string const & filename = m_query;
            auto fileSystem = Factories::CreateFileSystem();  // TODO: Use environment file system
            auto queries = ReadLines(*fileSystem, filename.c_str());
            const size_t c_threadCount = 8;
            const size_t c_iterations = 1;
            auto statistics =
                QueryRunner::Run(GetEnvironment().GetSimpleIndex(),
                                 GetEnvironment().GetOutputDir().c_str(),
                                 c_threadCount,
                                 queries,
                                 c_iterations);
            std::cout << "Results:" << std::endl;
            statistics.Print(std::cout);
        }
    }


    ICommand::Documentation Query::GetDocumentation()
    {
        return Documentation(
            "query",
            "Process a single query or list of queries.",
            "query (one <expression>) | (log <file>)\n"
            "  Processes a single query or a list of queries\n"
            "  specified by a file.\n"
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
        if (command.compare("cache") == 0)
        {
            m_mode = Mode::Cache;
            m_term = TaskFactory::GetNextToken(parameters);
        }
        else if (command.compare("rows") == 0)
        {
            m_mode = Mode::Rows;
            m_term = TaskFactory::GetNextToken(parameters);
        }
        else if (command.compare("term") == 0)
        {
            m_mode = Mode::Term;
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
        if (m_mode == Mode::Cache)
        {
            auto & environment = GetEnvironment();
            Term term(m_term.c_str(), 0, environment.GetConfiguration());
            auto & cache = environment.GetIngestor().GetDocumentCache();

            std::cout << "DocId, Contains" << std::endl;
            for (auto entry : cache)
            {
                std::cout
                    << "  DocId(" << entry.second << ") ";
                if (entry.first.Contains(term))
                {
                    std::cout << "contains ";
                }
                else
                {
                    std::cout << "does not contain ";
                }
                std::cout << m_term << std::endl;
            }
        }
        else
        {
            // TODO: Consider parsing phrase terms here.
            auto & environment = GetEnvironment();
            Term term(m_term.c_str(), 0, environment.GetConfiguration());
            RowIdSequence rows(term, environment.GetTermTable());

            std::cout
                << "Term("
                << "\"" << m_term << "\""
                << ")" << std::endl;

            IIngestor & ingestor = GetEnvironment().GetIngestor();


            // TODO: Come up with a better heuristic for deciding which
            // bits to display. Current algorithm is to display bits for
            // the first 64 documents with ids less than 1000.

            std::vector<DocId> ids;
            for (DocId id = 0; id <= 1000; ++id)
            {
                if (ingestor.Contains(id))
                {
                    ids.push_back(id);
                    if (ids.size() == 64)
                    {
                        break;
                    }
                }
            }

            // Print out 100s digit of DocId.
            std::cout << "                 d ";
            for (auto id : ids)
            {
                std::cout << id/100;
            }
            std::cout << std::endl;

            // Print ouf 10s digit of DocId.
            std::cout << "                 o ";
            for (auto id : ids)
            {
                std::cout << (id/10 % 10);
            }
            std::cout << std::endl;

            // Print out 1s digit of DocId.
            std::cout << "                 c ";
            for (auto id : ids)
            {
                std::cout << (id %10);
            }
            std::cout << std::endl;

            // Print out RowIds and their bits.
            for (auto row : rows)
            {
                std::cout
                    << "  RowId("
                    << row.GetRank()
                    << ", "
                    << std::setw(5)
                    << row.GetIndex()
                    << ")";

                if (m_mode == Mode::Rows)
                {
                    std::cout << ": ";
                    for (auto id : ids)
                    {
                        if (ingestor.Contains(id))
                        {
                            auto handle = ingestor.GetHandle(id);
                            std::cout << (handle.GetBit(row) ? "1" : "0");
                        }
                    }
                }

                std::cout << std::endl;
            }
        }
    }


    ICommand::Documentation Show::GetDocumentation()
    {
        return Documentation(
            "show",
            "Shows information about various data structures. (TODO)",
            "show cache <term>\n"
            "   | rows <term> [<docstart> <docend>]\n"
            "   | term <term>\n"
            //"   | shards\n"
            //"   | shard <shardid>\n"
            "  Shows information about various data structures."
            "  PARTIALLY IMPLEMENTED\n"
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

        GetEnvironment().GetIngestor().PrintStatistics(std::cout);

        std::cout << std::endl;

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


    //*************************************************************************
    //
    // Verify
    //
    //*************************************************************************
    Verify::Verify(Environment & environment,
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


    // TODO: this should be somewhere else.
    std::unique_ptr<IMatchVerifier> VerifyOneQuery
        (Environment & environment,
         std::string query)
    {
            auto streamConfiguration = Factories::CreateStreamConfiguration();
            QueryPipeline pipeline(*streamConfiguration);
            auto tree = pipeline.ParseQuery(query.c_str());

            std::unique_ptr<IMatchVerifier> verifier =
                Factories::CreateMatchVerifier(query);

            if (tree == nullptr)
            {
                // std::cout << "Empty query." << std::endl;
            }
            else
            {
                // auto & environment = GetEnvironment();
                auto & cache = environment.GetIngestor().GetDocumentCache();
                auto & config = environment.GetConfiguration();
                TermMatchTreeEvaluator evaluator(config);

                size_t matchCount = 0;
                size_t documentCount = 0;

                for (auto entry : cache)
                {
                    ++documentCount;
                    bool matches = evaluator.Evaluate(*tree, entry.first);

                    if (matches)
                    {
                        ++matchCount;
                        //std::cout
                        //    << "  DocId(" << entry.second << ") "
                        //    << std::endl;

                        verifier->AddExpected(entry.second);
                    }
                }

                // std::cout
                //     << matchCount << " match(es) out of "
                //     << documentCount << " documents."
                //     << std::endl;

                auto diagnosticStream = Factories::CreateDiagnosticStream(std::cout);
                // diagnosticStream->Enable("");

                QueryInstrumentation instrumentation;

                // auto observed = Factories::RunSimplePlanner(*tree,
                //                                             environment.GetSimpleIndex(),
                //                                             *diagnosticStream);
                auto observed = Factories::RunQueryPlanner(*tree,
                                                           environment.GetSimpleIndex(),
                                                           *diagnosticStream,
                                                           instrumentation);
                for (auto id : observed)
                {
                    verifier->AddObserved(id);
                }

                verifier->Verify();
                //verifier->Print(std::cout);
            }
            return verifier;
    }


    void Verify::Execute()
    {
        if (m_isSingleQuery)
        {
            std::cout
                << "Processing query \""
                << m_query
                << "\"" << std::endl;
            auto verifier = VerifyOneQuery(GetEnvironment(), m_query);
            std::cout << "True positives: "
                      << verifier->GetNumTruePositives()
                      << std::endl
                      << "False positives : "
                      << verifier->GetNumFalsePositives()
                      << std::endl;
            if (verifier->GetNumFalseNegatives() > 0)
            {
                throw RecoverableError("MatchVerifier: false negative detected.");
            }
        }
        else
        {
            std::cout
                << "Processing queries from log at \""
                << m_query
                << "\"" << std::endl;
            auto fileSystem = Factories::CreateFileSystem();  // TODO: Use environment file system
            auto queries = ReadLines(*fileSystem, m_query.c_str());

            // TODO: use FileManager.
            auto verificationOut = GetEnvironment().
                GetFileSystem().
                OpenForWrite("verificationOutput.csv");
            CsvTsv::CsvTableFormatter verificationFormatter(*verificationOut);
            CsvTsv::TableWriter writer(verificationFormatter);
            CsvTsv::OutputColumn<std::string>
                queryString("Query",
                            "Query text");
            CsvTsv::OutputColumn<uint64_t>
                docId("Document",
                      "ID of document.");
            CsvTsv::OutputColumn<uint64_t>
                type("Type",
                     "0: true positive. 1: false positive. 2: false negative. TODO: fix this");

            writer.DefineColumn(queryString);
            writer.DefineColumn(docId);
            writer.DefineColumn(type);
            writer.WritePrologue();

            auto verificationSummary = GetEnvironment().
                GetFileSystem().
                OpenForWrite("verificationSummary.csv");
            CsvTsv::CsvTableFormatter summaryFormatter(*verificationSummary);
            CsvTsv::TableWriter summary(summaryFormatter);
            CsvTsv::OutputColumn<uint64_t>
                termPos("TermPos",
                        "Term position.");
            CsvTsv::OutputColumn<uint64_t>
                numTruePos("TruePositives",
                           "Number of true positives.");
            CsvTsv::OutputColumn<uint64_t>
                numFalsePos("FalsePositives",
                            "Number of false positives.");
            CsvTsv::OutputColumn<uint64_t>
                numFalseNeg("FalseNegatives",
                           "Number of true negatives.");
            CsvTsv::OutputColumn<double>
                falseRate("FalseRate",
                          "(Num false positives) / (Num total matches).");


            summary.DefineColumn(queryString);
            summary.DefineColumn(termPos);
            summary.DefineColumn(numTruePos);
            summary.DefineColumn(numFalsePos);
            summary.DefineColumn(numFalseNeg);
            summary.DefineColumn(falseRate);
            summary.WritePrologue();

            uint64_t position = 0;
            for (const auto & query : queries)
            {
                auto verifier = VerifyOneQuery(GetEnvironment(), query);
                queryString = verifier->GetQuery();
                std::vector<DocId> results = verifier->GetTruePositives();
                numTruePos = results.size();
                type = 0;
                for (const auto id : results)
                {
                    docId = id;
                    writer.WriteDataRow();
                }

                results = verifier->GetFalsePositives();
                numFalsePos = results.size();
                type = 1;
                for (const auto id : results)
                {
                    docId = id;
                    writer.WriteDataRow();
                }

                results = verifier->GetFalseNegatives();
                numFalseNeg = results.size();
                type = 2;
                for (const auto id : results)
                {
                    docId = id;
                    writer.WriteDataRow();
                }

                falseRate = static_cast<double>(numFalsePos) /
                    (static_cast<double>(numFalsePos) +
                     static_cast<double>(numTruePos));

                summary.WriteDataRow();
                ++position;
                termPos = position;
            }

            writer.WriteEpilogue();
            summary.WriteEpilogue();
        }
    }


    ICommand::Documentation Verify::GetDocumentation()
    {
        return Documentation(
            "verify",
            "Verifies the results of a single query against the document cache.",
            "verify (one <expression>) | (log <file>)\n"
            "  Verifies a single query or a list of queries\n"
            "  against the document cache.\n"
            );
    }
}
