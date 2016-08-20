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
#include "BitFunnel/Index/IRecycler.h"
#include "BitFunnel/Index/ISliceBufferAllocator.h"
#include "BitFunnel/Row.h"
#include "SimpleIndex.h"


namespace BitFunnel
{
    std::unique_ptr<ISimpleIndex>
        Factories::CreateSimpleIndex(char const * directory,
                                     size_t gramSize,
                                     bool generateTermToText)
    {
        return std::unique_ptr<ISimpleIndex>(
            new SimpleIndex(directory, gramSize, generateTermToText));
    }


    SimpleIndex::SimpleIndex(char const * directory,
                             size_t gramSize,
                             bool generateTermToText)
        // TODO: Don't like passing *this to TaskFactory.
        // What if TaskFactory calls back before SimpleIndex is fully initialized?
        : m_directory(directory),
          m_gramSize(static_cast<Term::GramSize>(gramSize)),
          m_generateTermToText(generateTermToText)
    {
    }


    void SimpleIndex::StartIndex(bool forStatistics)
    {
        char const * directory = m_directory.c_str();
        m_fileManager = Factories::CreateFileManager(directory,
                                                     directory,
                                                     directory);

        m_schema = Factories::CreateDocumentDataSchema();

        m_recycler = Factories::CreateRecycler();
        m_recyclerThread = std::thread(RecyclerThreadEntryPoint, this);


        // TODO: Load shard definition from FileManager stream.
        // TODO: Optimal shard.
        m_shardDefinition = Factories::CreateShardDefinition();
        // m_shardDefinition->AddShard(1000);
        // m_shardDefinition->AddShard(2000);
        // m_shardDefinition->AddShard(3000);

        // Load the TermTables
        {
            if (forStatistics)
            {
                m_termTables =
                    Factories::CreateTermTableCollection(m_shardDefinition->GetShardCount());
            }
            else
            {
                m_termTables =
                    Factories::CreateTermTableCollection(*m_fileManager,
                                                         m_shardDefinition->GetShardCount());
            }
        }

        // Load the IndexedIdfTable
        {
            auto input = m_fileManager->IndexedIdfTable(0).OpenForRead();
            Term::IdfX10 defaultIdf = 60;   // TODO: use proper value here.
            m_idfTable = Factories::CreateIndexedIdfTable(*input, defaultIdf);
        }

        m_configuration =
            Factories::CreateConfiguration(m_gramSize, m_generateTermToText, *m_idfTable);

        // TODO: Need a blockSize that works for all term tables.
        const ShardId tempId = 0;
        const size_t blockSize =
            GetMinimumBlockSize(*m_schema, m_termTables->GetTermTable(tempId));
        std::cout << "Blocksize: " << blockSize << std::endl;

        const size_t initialBlockCount = 16;
        m_sliceAllocator = Factories::CreateSliceBufferAllocator(blockSize,
                                                                 initialBlockCount);

        m_ingestor = Factories::CreateIngestor(*m_schema,
                                               *m_recycler,
                                               *m_termTables,
                                               *m_shardDefinition,
                                               *m_sliceAllocator);
    }


    void SimpleIndex::StopIndex()
    {
        m_recycler->Shutdown();
        m_recyclerThread.join();
        m_ingestor->Shutdown();
    }


    IConfiguration const & SimpleIndex::GetConfiguration() const
    {
        return *m_configuration;
    }


    IFileManager & SimpleIndex::GetFileManager() const
    {
        return *m_fileManager;
    }


    IIngestor & SimpleIndex::GetIngestor() const
    {
        return *m_ingestor;
    }


    IRecycler & SimpleIndex::GetRecycler() const
    {
        return *m_recycler;
    }


    ITermTable2 const & SimpleIndex::GetTermTable() const
    {
        // TODO: There is a different TermTable in each shard. Which should
        // be returned? Currently returning the TermTable for shard 0.
        const ShardId tempId = 0;
        return m_termTables->GetTermTable(tempId);
    }


    void SimpleIndex::RecyclerThreadEntryPoint(void * data)
    {
        SimpleIndex* index = reinterpret_cast<SimpleIndex*>(data);
        index->m_recycler->Run();
    }
}
