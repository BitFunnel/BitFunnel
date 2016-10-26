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

#include "Allocator.h"
#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Configuration/IStreamConfiguration.h"
#include "BitFunnel/IDiagnosticStream.h"
#include "BitFunnel/Plan/Factories.h"
#include "BitFunnel/Plan/QueryRunner.h"
#include "BitFunnel/Utilities/Factories.h"
#include "BitFunnel/Utilities/ITaskDistributor.h"
#include "BitFunnel/Utilities/Stopwatch.h"
#include "QueryParser.h"


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


    //QueryRunner::QueryRunner(ISimpleIndex const & index,
    //                         size_t threadCount)
    //    : m_index(index),
    //      m_threadCount(threadCount)
    //{
    //}


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
                       std::vector<std::string> const & queries);

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

        std::unique_ptr<IAllocator> m_allocator;

        static const size_t c_allocatorSize = 16384;
    };

    QueryProcessor::QueryProcessor(ISimpleIndex const & index,
                                   IStreamConfiguration const & config,
                                   std::vector<std::string> const & queries)
      : m_index(index),
        m_config(config),
        m_queries(queries),
        m_allocator(new Allocator(c_allocatorSize))
    {
    }


    void QueryProcessor::ProcessTask(size_t taskId)
    {
        m_allocator->Reset();

        size_t queryId = taskId % m_queries.size();

        // TODO: Shouldn't use an std::stringstream here.
        // Just causes an extra copy and allocation per query.
        QueryParser parser(m_queries[queryId].c_str(), m_config, *m_allocator);
        auto tree = parser.Parse();

        // TODO: remove diagnosticStream and replace with nullable.
        auto diagnosticStream = Factories::CreateDiagnosticStream(std::cout);
        if (tree != nullptr)
        {
            auto observed = Factories::RunSimplePlanner(*tree,
                                                        m_index,
                                                        *diagnosticStream);
        }
    }


    void QueryProcessor::Finished()
    {
    }

    //*************************************************************************
    //
    // QueryRunner
    //
    //*************************************************************************
    QueryRunner::Statistics QueryRunner::Run(
        ISimpleIndex const & index,
        size_t threadCount,
        std::vector<std::string> const & queries,
        size_t iterations)
    {
        auto config = Factories::CreateStreamConfiguration();

        std::vector<std::unique_ptr<ITaskProcessor>> processors;
        for (size_t i = 0; i < threadCount; ++i) {
            processors.push_back(
                std::unique_ptr<ITaskProcessor>(
                    new QueryProcessor(index, *config, queries)));
        }

        auto distributor =
            Factories::CreateTaskDistributor(processors,
                                             queries.size() * iterations);

        Stopwatch stopwatch;
        distributor->WaitForCompletion();

        return QueryRunner::Statistics(threadCount,
                                       queries.size(),
                                       queries.size() * iterations,
                                       stopwatch.ElapsedTime());
    }
}
