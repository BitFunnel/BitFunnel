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
#include "BitFunnel/Configuration/IStreamConfiguration.h"
#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Plan/Factories.h"
#include "BitFunnel/Plan/IQueryEngine.h"
#include "BitFunnel/Plan/QueryInstrumentation.h"
#include "BitFunnel/Plan/QueryRunner.h"
#include "BitFunnel/Plan/ResultsBuffer.h"
#include "BitFunnel/Utilities/ReadLines.h"
#include "CsvTsv/Csv.h"
#include "Environment.h"
#include "LoggerInterfaces/Check.h"
#include "QueryCommand.h"


namespace BitFunnel
{
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
            m_queryCommand = QueryOne;
            m_query = parameters;
        }
        else if (command.compare("docs") == 0)
        {
            m_queryCommand = QueryDocs;
            m_query = parameters;
        }
        else
        {
            m_queryCommand = QueryLog;
            if (command.compare("log") != 0)
            {
                std::stringstream message;
                message << "expected log or one" << std::endl;
                throw RecoverableError(message.str().c_str());
            }
            m_query = TaskFactory::GetNextToken(parameters);
        }
    }


    void Query::Execute()
    {
        std::ostream& output = GetEnvironment().GetOutputStream();

        try
        {
            if (m_queryCommand == QueryOne)
            {
                output
                    << "Processing query \""
                    << m_query
                    << "\"" << std::endl;
                auto instrumentation =
                    QueryRunner::Run(m_query.c_str(),
                        GetEnvironment().GetSimpleIndex(),
                        GetEnvironment().GetCompilerMode(),
                        GetEnvironment().GetCacheLineCountMode());

                output << "Results:" << std::endl;
                CsvTsv::CsvTableFormatter formatter(output);
                QueryInstrumentation::Data::FormatHeader(formatter);
                instrumentation.Format(formatter);
            }
            else if (m_queryCommand == QueryDocs)
            {
                output
                    << "Processing query \""
                    << m_query
                    << "\"" << std::endl;

                BitFunnel::QueryInstrumentation instrumentation;
                auto resultsBuffer = BitFunnel::ResultsBuffer(GetEnvironment().GetSimpleIndex().GetIngestor().GetDocumentCount());
                auto streammap = BitFunnel::Factories::CreateStreamConfiguration();
                auto queryEngine = BitFunnel::Factories::CreateQueryEngine(GetEnvironment().GetSimpleIndex(), *streammap);
                auto tree = queryEngine->Parse(m_query.c_str());
                instrumentation.FinishParsing();
                if (tree != nullptr)
                {
                    queryEngine->Run(tree, instrumentation, resultsBuffer);
                }

                output << "Results:" << std::endl;
                CsvTsv::CsvTableFormatter formatter(output);
                QueryInstrumentation::Data::FormatHeader(formatter);
                instrumentation.GetData().Format(formatter);

                output << std::endl << "Document Ids" << std::endl;
                for (auto result : resultsBuffer)
                {
                    output << result.GetHandle().GetDocId() << std::endl;
                }

            }
            else
            {
                CHECK_NE(*GetEnvironment().GetOutputDir().c_str(), '\0')
                    << "Output directory not set. "
                    << "Please use the 'cd' command to set an "
                    << "output directory";

                output
                    << "Processing queries from log at \""
                    << m_query
                    << "\"" << std::endl;

                std::string const & filename = m_query;
                auto fileSystem = Factories::CreateFileSystem();  // TODO: Use environment file system
                auto queries = ReadLines(*fileSystem, filename.c_str());
                const size_t c_threadCount = GetEnvironment().GetThreadCount();
                const size_t c_iterations = 1;
                auto statistics =
                    QueryRunner::Run(GetEnvironment().GetSimpleIndex(),
                        GetEnvironment().GetOutputDir().c_str(),
                        c_threadCount,
                        queries,
                        c_iterations,
                        GetEnvironment().GetCompilerMode(),
                        GetEnvironment().GetCacheLineCountMode());
                output << "Results:" << std::endl;
                statistics.Print(output);

                // TODO: unify this with the fileManager that's passed into
                // QueryRunner::Run.
                auto outFileManager =
                    Factories::CreateFileManager(GetEnvironment().GetOutputDir().c_str(),
                        GetEnvironment().GetOutputDir().c_str(),
                        GetEnvironment().GetOutputDir().c_str(),
                        GetEnvironment().GetSimpleIndex().GetFileSystem());

                auto outSummary = outFileManager->QuerySummaryStatistics().OpenForWrite();
                statistics.Print(*outSummary);
            }
        }
        catch (RecoverableError e)
        {
            output << "Error: " << e.what() << std::endl;
        }
        catch (Logging::CheckException e)
        {
            output << "Error: " << e.GetMessage().c_str() << std::endl;
        }
    }


    ICommand::Documentation Query::GetDocumentation()
    {
        return Documentation(
            "query",
            "Process a single query or list of queries.",
            "query (one <query>) | (docs <query>) | (log <file>)\n"
            "  Processes a single query or a list of queries\n"
            "  specified by a file.\n"
            "  'docs' lists all matching documents."
        );
    }
}
