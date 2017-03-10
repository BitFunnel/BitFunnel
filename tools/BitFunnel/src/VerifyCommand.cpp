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

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Plan/IMatchVerifier.h"
#include "BitFunnel/Plan/VerifyOneQuery.h"
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
            m_isOutput = false;
            m_query = parameters;
        }
        else if (command.compare("log") == 0)
        {
            m_isSingleQuery = false;
            m_isOutput = false;
            m_query = TaskFactory::GetNextToken(parameters);
        }
        else if (command.compare("output") == 0)
        {
            m_isSingleQuery = false;
            m_isOutput = true;
            m_query = TaskFactory::GetNextToken(parameters);
        }
        else
        {
            RecoverableError error("Query expects \"one\" or \"log\".");
            throw error;
        }
    }


    void Verify::Execute()
    {
        std::ostream& output = GetEnvironment().GetOutputStream();

        if (m_isSingleQuery)
        {
            output
                << "Processing query \""
                << m_query
                << "\"" << std::endl;
            auto verifier = VerifyOneQuery(GetEnvironment().GetSimpleIndex(),
                                           m_query,
                                           true,
                                           GetEnvironment().GetCompilerMode());
            output << "True positive count: "
                << verifier->GetTruePositiveCount()
                << std::endl
                << "False positive count: "
                << verifier->GetFalsePositiveCount()
                << std::endl
                << "False negative count: "
                << verifier->GetFalseNegativeCount()
                << std::endl;
            if (verifier->GetTruePositiveCount() > 0)
            {
                output << "True positives: ";
                size_t counter = 0;
                for (const auto doc : verifier->GetTruePositives())
                {
                    if (counter != 0)
                    {
                        output << ", ";
                    }
                    output << doc;
                    ++counter;
                }
                output << std::endl;
                output << std::endl;
            }

            if (verifier->GetFalsePositiveCount() > 0)
            {
                output << "False positives:" << std::endl;
                for (const auto doc : verifier->GetFalsePositives())
                {
                    output << doc << ",";
                }
                output << std::endl;
            }

            if (verifier->GetFalseNegativeCount() > 0)
            {
                output << "False negatives:" << std::endl;
                for (const auto doc : verifier->GetFalseNegatives())
                {
                    output << doc << ",";
                }
                output << std::endl;

                throw RecoverableError("MatchVerifier: false negative detected.");
            }
        }
        else
        {
            output
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
                     "0: true positive. 1: false positive. 2: false negative. 3: unchecked result. TODO: fix this");

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
                auto verifier = VerifyOneQuery(GetEnvironment().GetSimpleIndex(),
                                               query,
                                               !m_isOutput,
                                               GetEnvironment().GetCompilerMode());
                queryString = verifier->GetQuery();

                if (!m_isOutput)
                {
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
                else
                {
                    // TODO: fix this confusing setup. In m_isOutput mode, we
                    // never run the verifier and therefore never have
                    // "expected" results, which means that everything shows up
                    // as a false positive. This terminology conflict is because
                    // we added this mode after creating the verifier.
                    std::vector<DocId> results = verifier->GetFalsePositives();
                    type = 3;
                    for (const auto id : results)
                    {
                        docId = id;
                        writer.WriteDataRow();
                    }

                }
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
