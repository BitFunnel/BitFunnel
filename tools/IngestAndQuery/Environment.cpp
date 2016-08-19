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
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/Helpers.h"
#include "BitFunnel/Index/ISliceBufferAllocator.h"
#include "BitFunnel/Row.h"
#include "Commands.h"
#include "Environment.h"
#include "TaskPool.h"


namespace BitFunnel
{
    Environment::Environment(char const * directory,
                             size_t gramSize,
                             size_t threadCount)
        // TODO: Don't like passing *this to TaskFactory.
        // What if TaskFactory calls back before Environment is fully initialized?
        : m_directory(directory),
          m_gramSize(static_cast<Term::GramSize>(gramSize)),
          m_taskFactory(new TaskFactory(*this))
    {
        // Start one extra thread for the Recycler.
        m_taskPool.reset(new TaskPool(threadCount + 1));

        RegisterCommands();

    }


    TaskFactory & Environment::GetTaskFactory() const
    {
        return *m_taskFactory;
    }


    TaskPool & Environment::GetTaskPool() const
    {
        return *m_taskPool;
    }


    void Environment::RegisterCommands()
    {
        m_taskFactory->RegisterCommand<DelayedPrint>();
        m_taskFactory->RegisterCommand<Exit>();
        m_taskFactory->RegisterCommand<Help>();
        m_taskFactory->RegisterCommand<Ingest>();
        m_taskFactory->RegisterCommand<Query>();
        m_taskFactory->RegisterCommand<Script>();
        m_taskFactory->RegisterCommand<Show>();
        m_taskFactory->RegisterCommand<Status>();
    }


    class RecyclerTask : public ITask
    {
    public:
        RecyclerTask(IRecycler & recycler)
          : m_recycler(recycler)
        {
        }

        virtual void Execute() override
        {
            m_recycler.Run();
        }

    private:
        IRecycler & m_recycler;
    };


    void Environment::StartIndex()
    {
        char const * directory = m_directory.c_str();
        m_fileManager = Factories::CreateFileManager(directory,
                                                     directory,
                                                     directory);

        m_schema = Factories::CreateDocumentDataSchema();

        m_recycler = Factories::CreateRecycler();
        m_taskPool->TryEnqueue(
            std::unique_ptr<RecyclerTask>(new RecyclerTask(*m_recycler)));


        // Load the TermTable
        {
            auto input = m_fileManager->TermTable(0).OpenForRead();
            m_termTable = Factories::CreateTermTable(*input);
        }

        // Load the IndexedIdfTable
        {
            auto input = m_fileManager->IndexedIdfTable(0).OpenForRead();
            Term::IdfX10 defaultIdf = 60;   // TODO: use proper value here.
            m_idfTable = Factories::CreateIndexedIdfTable(*input, defaultIdf);
        }

        m_configuration =
            Factories::CreateConfiguration(m_gramSize, false, *m_idfTable);

        const size_t blockSize = GetMinimumBlockSize(*m_schema, *m_termTable);
        std::cout << "Blocksize: " << blockSize << std::endl;

        const size_t initialBlockCount = 16;
        m_sliceAllocator = Factories::CreateSliceBufferAllocator(blockSize,
                                                                 initialBlockCount);

        //auto shardDefinition = Factories::CreateShardDefinition();
        //// shardDefinition->AddShard(1000);
        //// shardDefinition->AddShard(2000);
        //// shardDefinition->AddShard(3000);

        //const std::unique_ptr<IIngestor>
        //    ingestor(Factories::CreateIngestor(*fileManager,
        //                                       schema,
        //                                       *recycler,
        //                                       *termTable,
        //                                       *shardDefinition,
        //                                       *sliceAllocator));

    }


    void Environment::StopIndex()
    {
        m_recycler->Shutdown();
    }


    ITermTable2 const & Environment::GetTermTable() const
    {
        return *m_termTable;
    }


    IConfiguration const & Environment::GetConfiguration() const
    {
        return *m_configuration;
    }
}
