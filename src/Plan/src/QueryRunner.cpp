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

#include <condition_variable>
#include <iostream>             // Used for DiagnosticStream ref; not actually used.

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Configuration/IStreamConfiguration.h"
#include "BitFunnel/IDiagnosticStream.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Plan/Factories.h"
#include "BitFunnel/Plan/QueryInstrumentation.h"
#include "BitFunnel/Plan/IQueryEngine.h"
#include "BitFunnel/Plan/QueryParser.h"
#include "BitFunnel/Plan/QueryRunner.h"
#include "BitFunnel/Plan/ResultsBuffer.h"
#include "BitFunnel/Utilities/Factories.h"
#include "BitFunnel/Utilities/Allocator.h"
#include "ByteCodeQueryEngine.h"
#include "CsvTsv/Csv.h"
#include "NativeJITQueryEngine.h"


namespace BitFunnel
{
    QueryRunner::Statistics::Statistics(
        size_t threadCount,
        size_t uniqueQueryCount,
        size_t processedCount,
        size_t matchCount,
        double elapsedTime,
        double parsingTime,
        double planningTime,
        double matchingTime)
      : m_threadCount(threadCount),
        m_uniqueQueryCount(uniqueQueryCount),
        m_processedCount(processedCount),
        m_matchCount(matchCount),
        m_elapsedTime(elapsedTime),
        m_parsingLatency(parsingTime),
        m_planningLatency(planningTime),
        m_matchingLatency(matchingTime)
    {
    }


    void QueryRunner::Statistics::Print(std::ostream& out) const
    {
        double totalLatency = m_parsingLatency + m_planningLatency + m_matchingLatency;
        double overheadLatency = m_parsingLatency + m_planningLatency;

        if (m_uniqueQueryCount != m_processedCount)
        {
            out << "WARNING: Not all queries were processed; some failed or are invalid" << std::endl;
        }

        out
            << "Index type: BitFunnel" << std::endl
            << "Thread count: " << m_threadCount << std::endl
            << "Unique queries: " << m_uniqueQueryCount << std::endl
            << "Queries processed: " << m_processedCount << std::endl
            << "Match count: " << m_matchCount << std::endl
            << "Elapsed time: " << m_elapsedTime << std::endl
            << "Total parsing latency: " << m_parsingLatency << std::endl
            << "Total planning latency: " << m_planningLatency << std::endl
            << "Total matching latency: " << m_matchingLatency << std::endl
            << "Mean query latency: " << totalLatency / m_processedCount << std::endl
            << "Planning overhead: " << overheadLatency / totalLatency << std::endl
            << "QPS: " << m_processedCount / m_elapsedTime << std::endl
            << "MPS: " << m_matchCount / m_elapsedTime << std::endl
            << "MPQ: " << static_cast<double>(m_matchCount) / m_processedCount << std::endl;
    }


    //*************************************************************************
    //
    // ThreadSynchronizer
    //
    // ThreadSynchronizer reduces the lag between the times various threads
    // start to do work. Intended for aiding in performance measurement where
    // we don't want to capture the OS thread startup time.
    //
    // Suspends the first n - 1 threads to call Wait().
    // Wakes all threads on the nth call to Wait().
    //
    //*************************************************************************
    class ThreadSynchronizer
    {
    public:
        ThreadSynchronizer(size_t threadCount);

        void Wait();

        double GetElapsedTime() const;

    private:
        size_t m_threadCount;
        std::mutex m_lock;
        std::condition_variable m_wakeCondition;

        Stopwatch m_stopwatch;
    };


    ThreadSynchronizer::ThreadSynchronizer(size_t threadCount)
        : m_threadCount(threadCount)
    {
    }


    void ThreadSynchronizer::Wait()
    {
        std::unique_lock<std::mutex> lock(m_lock);

        --m_threadCount;
        if (m_threadCount == 0)
        {
            m_stopwatch.Reset();
            m_wakeCondition.notify_all();
        }
        else
        {
            while (m_threadCount > 0)
            {
                m_wakeCondition.wait(lock);
            }
        }
    }


    double ThreadSynchronizer::GetElapsedTime() const
    {
        return m_stopwatch.ElapsedTime();
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
                       size_t maxResultCount,
                       bool useNativeCode,
                       bool countCacheLines,
                       ThreadSynchronizer& synchronizer);

        //
        // ITaskProcessor methods
        //

        virtual void ProcessTask(size_t taskId) override;
        virtual void Finished() override;

    private:
        //
        // constructor parameters
        //
        std::vector<std::string> const & m_queries;
        std::vector<QueryInstrumentation::Data> & m_results;
        ThreadSynchronizer& m_synchronizer;

        std::vector<ResultsBuffer::Result> m_matches;

        ResultsBuffer m_resultsBuffer;

        std::unique_ptr<IQueryEngine> m_queryEngine;

        size_t m_queriesProcessed;

        // TODO: Issue #390. Trec 2006 Efficiency Topic 43860 is too bit for
        // c_allocatorSize == 1ull << 17 when using TreatmentClassicBitsliced:
        //     the nps air quality monitoring program provides information on ozone
        //     levels acid rain and visibility impairment in parks from 1990 1999
        //     of the 28 parks that were monitored for visibility
        static const size_t c_allocatorSize = 1ull << 17;
    };


    QueryProcessor::QueryProcessor(ISimpleIndex const & index,
                                   IStreamConfiguration const & config,
                                   std::vector<std::string> const & queries,
                                   std::vector<QueryInstrumentation::Data> & results,
                                   size_t maxResultCount,
                                   bool useNativeCode,
                                   bool countCacheLines,
                                   ThreadSynchronizer& synchronizer)
      : m_queries(queries),
        m_results(results),
        m_synchronizer(synchronizer),
        m_matches(maxResultCount, {nullptr, 0}),
        m_resultsBuffer(index.GetIngestor().GetDocumentCount()),
        m_queriesProcessed(0)
    {
        if (useNativeCode)
        {
            m_queryEngine = std::unique_ptr<IQueryEngine>(new NativeJITQueryEngine(index, config, c_allocatorSize, c_allocatorSize));
        }
        else
        {
            m_queryEngine = std::unique_ptr<IQueryEngine>(new ByteCodeQueryEngine(index, config, c_allocatorSize));
        }

        if (countCacheLines)
        {
            m_queryEngine->EnableDiagnostic("planning/countcachelines");
        }
    }


    void QueryProcessor::ProcessTask(size_t taskId)
    {
        // If this is the first query, wait for other threads before continuing.
        if (m_queriesProcessed == 0)
        {
            m_synchronizer.Wait();
        }
        ++m_queriesProcessed;

        QueryInstrumentation instrumentation;

        size_t queryId = taskId % m_queries.size();

        // Parse and run the query, catching ParseError or other RecoverableError
        try
        {
            auto tree = m_queryEngine->Parse(m_queries[queryId].c_str());
            instrumentation.FinishParsing();

            if (tree != nullptr)
            {
                m_queryEngine->Run(tree,
                                   instrumentation,
                                   m_resultsBuffer);
            }
        }
        catch (RecoverableError e)
        {
            // Continue processing other queries, even though the current query failed.
            // The instrumentation for this query will show that it didn't succeed.
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
        ISimpleIndex const & index,
        bool useNativeCode,
        bool countCacheLines)
    {
        std::vector<std::string> queries;
        queries.push_back(std::string(query));

        std::vector<QueryInstrumentation::Data> results(queries.size());

        auto config = Factories::CreateStreamConfiguration();

        size_t maxResultCount = index.GetIngestor().GetDocumentCount();

        ThreadSynchronizer synchronizer(1);

        QueryProcessor
            processor(index,
                      *config,
                      queries,
                      results,
                      maxResultCount,
                      useNativeCode,
                      countCacheLines,
                      synchronizer);
        processor.ProcessTask(0);
        processor.Finished();

        return results[0];
    }


    QueryRunner::Statistics QueryRunner::Run(
        ISimpleIndex const & index,
        char const * outDir,
        size_t threadCount,
        std::vector<std::string> const & queries,
        size_t iterations,
        bool useNativeCode,
        bool countCacheLines)
    {
        std::vector<QueryInstrumentation::Data> results(queries.size() * iterations);

        auto config = Factories::CreateStreamConfiguration();

        size_t maxResultCount = index.GetIngestor().GetDocumentCount();

        ThreadSynchronizer synchronizer(threadCount);

        std::vector<std::unique_ptr<ITaskProcessor>> processors;
        for (size_t i = 0; i < threadCount; ++i) {
            processors.push_back(
                std::unique_ptr<ITaskProcessor>(
                    new QueryProcessor(index,
                                       *config,
                                       queries,
                                       results,
                                       maxResultCount,
                                       useNativeCode,
                                       countCacheLines,
                                       synchronizer)));
        }

        auto distributor =
            Factories::CreateTaskDistributor(processors,
                                             queries.size() * iterations);

        distributor->WaitForCompletion();
        double elapsedTime = synchronizer.GetElapsedTime();

        double totalParsingTime = 0;
        double totalPlanningTime = 0;
        double totalMatchingTime = 0;

        size_t queriesProcessed = 0;
        size_t matchCount = 0;
        for (auto result : results)
        {
            if (result.GetSucceeded())
            {
                ++queriesProcessed;
                matchCount += result.GetMatchCount();
                totalParsingTime += result.GetParsingTime();
                totalPlanningTime += result.GetPlanningTime();
                totalMatchingTime += result.GetMatchingTime();
            }
        }

        auto statistics(QueryRunner::Statistics(threadCount,
                                                queries.size(),
                                                queriesProcessed,
                                                matchCount,
                                                elapsedTime,
                                                totalParsingTime,
                                                totalPlanningTime,
                                                totalMatchingTime));

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
