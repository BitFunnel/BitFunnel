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

#include "BitFunnel/Allocators/IAllocator.h"
#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/Configuration/IStreamConfiguration.h"
#include "BitFunnel/Exceptions.h"
#include "BitFunnel/IDiagnosticStream.h"
#include "BitFunnel/Index/IDocumentCache.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Plan/Factories.h"
#include "BitFunnel/Plan/IMatchVerifier.h"
#include "BitFunnel/Plan/QueryInstrumentation.h"
#include "BitFunnel/Plan/QueryParser.h"
#include "BitFunnel/Plan/TermMatchTreeEvaluator.h"
#include "BitFunnel/Utilities/Factories.h"
#include "BitFunnel/Utilities/ReadLines.h"
#include "CsvTsv/Csv.h"
#include "Environment.h"
#include "VerifyCommand.h"


namespace BitFunnel
{
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
    std::unique_ptr<IMatchVerifier> VerifyOneQuery(
        Environment & environment,
        std::string query)
    {
        auto streamConfiguration = Factories::CreateStreamConfiguration();
        auto allocator = Factories::CreateAllocator(4096 * 64);

        QueryParser parser(query.c_str(), *streamConfiguration, *allocator);
        auto tree = parser.Parse();

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
                << verifier->GetTruePositiveCount()
                << std::endl
                << "False positives : "
                << verifier->GetFalsePositiveCount()
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
