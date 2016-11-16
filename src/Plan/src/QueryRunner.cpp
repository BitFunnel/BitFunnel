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

#include <iostream>  // Used for DiagnosticStream ref; not actually used.
#include <memory>  // Used for std::unique_ptr of diagnosticStream. Probably temporary.
#include <ostream>

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Configuration/IStreamConfiguration.h"
#include "BitFunnel/IDiagnosticStream.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Plan/Factories.h"
#include "BitFunnel/Plan/QueryInstrumentation.h"
#include "BitFunnel/Plan/QueryParser.h"
#include "BitFunnel/Plan/QueryRunner.h"
#include "BitFunnel/Plan/ResultsBuffer.h"
#include "BitFunnel/Utilities/Factories.h"
#include "BitFunnel/Utilities/Allocator.h"
#include "BitFunnel/Utilities/ITaskDistributor.h"
#include "BitFunnel/Utilities/Stopwatch.h"
#include "CsvTsv/Csv.h"


namespace BitFunnel
{
    QueryRunner::Statistics::Statistics(
        size_t threadCount,
        size_t uniqueQueryCount,
        size_t processedCount,
        double elapsedTime)
      : m_threadCount(threadCount),
        m_uniqueQueryCount(uniqueQueryCount),
        m_processedCount(processedCount),
        m_elapsedTime(elapsedTime)
    {
    }

    void QueryRunner::Statistics::Print(std::ostream& out) const
    {
        out << "Print" << std::endl;
        out
            << "Thread count: " << m_threadCount << std::endl
            << "Unique queries: " << m_uniqueQueryCount << std::endl
            << "Queries processed: " << m_processedCount << std::endl
            << "Elapsed time: " << m_elapsedTime << std::endl
            << "QPS: " << m_processedCount / m_elapsedTime << std::endl;
    }


    //*************************************************************************
    //
    // QueryProcessor
    //
    //*************************************************************************
    class QueryProcessor : public ITaskProcessor
    {
    public:
        QueryProcessor(ISimpleIndex const & index,
                       IStreamConfiguration const & config,
                       std::vector<std::string> const & queries,
                       std::vector<QueryInstrumentation::Data> & results,
                       size_t maxResultCount);

        static QueryInstrumentation ProcessOneQuery(
            char const * query,
            ISimpleIndex const & index,
            IStreamConfiguration const & config,
            IAllocator & allocator,
            IDiagnosticStream & diagnosticStream);

        //
        // ITaskProcessor methods
        //

        virtual void ProcessTask(size_t taskId) override;
        virtual void Finished() override;

    private:
        //
        // constructor parameters
        //
        ISimpleIndex const & m_index;
        IStreamConfiguration const & m_config;
        std::vector<std::string> const & m_queries;
        std::vector<QueryInstrumentation::Data> & m_results;
        std::vector<ResultsBuffer::Result> m_matches;

        ResultsBuffer m_resultsBuffer;

        std::unique_ptr<IAllocator> m_allocator;

        static const size_t c_allocatorSize = 1ull << 16;
    };


    QueryProcessor::QueryProcessor(ISimpleIndex const & index,
                                   IStreamConfiguration const & config,
                                   std::vector<std::string> const & queries,
                                   std::vector<QueryInstrumentation::Data> & results,
                                   size_t maxResultCount)
      : m_index(index),
        m_config(config),
        m_queries(queries),
        m_results(results),
        m_matches(maxResultCount, {nullptr, 0}),
        m_resultsBuffer(index.GetIngestor().GetDocumentCount()),
        m_allocator(new Allocator(c_allocatorSize))
    {
    }


    void QueryProcessor::ProcessTask(size_t taskId)
    {
        QueryInstrumentation instrumentation;
        m_allocator->Reset();

        size_t queryId = taskId % m_queries.size();

        QueryParser parser(m_queries[queryId].c_str(), m_config, *m_allocator);
        auto tree = parser.Parse();
        instrumentation.FinishParsing();

        // TODO: remove diagnosticStream and replace with nullable.
        auto diagnosticStream = Factories::CreateDiagnosticStream(std::cout);
        if (tree != nullptr)
        {
            Factories::RunQueryPlanner(*tree,
                                       m_index,
                                       *m_allocator,
                                       *diagnosticStream,
                                       instrumentation,
                                       m_resultsBuffer,
                                       false);
        }

        m_results[taskId] = instrumentation.GetData();
    }


    void QueryProcessor::Finished()
    {
    }

    //*************************************************************************
    //
    // QueryRunner
    //
    //*************************************************************************
    QueryInstrumentation::Data QueryRunner::Run(
        char const * query,
        ISimpleIndex const & index)
    {
        std::vector<std::string> queries;
        queries.push_back(std::string(query));

        std::vector<QueryInstrumentation::Data> results(queries.size());

        auto config = Factories::CreateStreamConfiguration();

        size_t maxResultCount = index.GetIngestor().GetDocumentCount();

        QueryProcessor
            processor(index, *config, queries, results, maxResultCount);
        processor.ProcessTask(0);
        processor.Finished();

        return results[0];
    }


    QueryRunner::Statistics QueryRunner::Run(
        ISimpleIndex const & index,
        char const * outDir,
        size_t threadCount,
        std::vector<std::string> const & queries,
        size_t iterations)
    {
        std::vector<QueryInstrumentation::Data> results(queries.size() * iterations);

        auto config = Factories::CreateStreamConfiguration();

        size_t maxResultCount = index.GetIngestor().GetDocumentCount();

        std::vector<std::unique_ptr<ITaskProcessor>> processors;
        for (size_t i = 0; i < threadCount; ++i) {
            processors.push_back(
                std::unique_ptr<ITaskProcessor>(
                    new QueryProcessor(index,
                                       *config,
                                       queries,
                                       results,
                                       maxResultCount)));
        }

        auto distributor =
            Factories::CreateTaskDistributor(processors,
                                             queries.size() * iterations);

        Stopwatch stopwatch;
        distributor->WaitForCompletion();

        auto statistics(QueryRunner::Statistics(threadCount,
                                                queries.size(),
                                                queries.size() * iterations,
                                                stopwatch.ElapsedTime()));

        {
            std::cout << "Writing results ..." << std::endl;
            auto outFileManager =
                Factories::CreateFileManager(outDir,
                                             outDir,
                                             outDir,
                                             index.GetFileSystem());

            auto out = outFileManager->QueryPipelineStatistics().OpenForWrite();
            CsvTsv::CsvTableFormatter formatter(*out);

            formatter.WriteField("query");
            QueryInstrumentation::Data::FormatHeader(formatter);
            for (size_t i = 0; i < results.size(); ++i)
            {
                formatter.WriteField(queries[i % queries.size()]);
                results[i].Format(formatter);
            }
        }

        return statistics;
    }
}
