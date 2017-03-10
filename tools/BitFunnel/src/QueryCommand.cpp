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
#include "BitFunnel/Plan/QueryInstrumentation.h"
#include "BitFunnel/Plan/QueryRunner.h"
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
        std::ostream& output = GetEnvironment().GetOutputStream();

        if (m_isSingleQuery)
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
}
