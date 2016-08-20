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
#include "SimpleIndex.h"


namespace BitFunnel
{
    std::unique_ptr<ISimpleIndex>
        Factories::CreateSimpleIndex(char const * directory,
                                     size_t gramSize)
    {
        return std::unique_ptr<ISimpleIndex>(
            new SimpleIndex(directory, gramSize));
    }


    SimpleIndex::SimpleIndex(char const * directory,
                             size_t gramSize)
        // TODO: Don't like passing *this to TaskFactory.
        // What if TaskFactory calls back before SimpleIndex is fully initialized?
        : m_directory(directory),
          m_gramSize(static_cast<Term::GramSize>(gramSize))
    {
    }


    void SimpleIndex::StartIndex()
    {
        char const * directory = m_directory.c_str();
        m_fileManager = Factories::CreateFileManager(directory,
                                                     directory,
                                                     directory);

        m_schema = Factories::CreateDocumentDataSchema();

        m_recycler = Factories::CreateRecycler();


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

        // TODO: Load shard definition from FileManager stream.
        // TODO: Optimal shard.
        m_shardDefinition = Factories::CreateShardDefinition();
        // m_shardDefinition->AddShard(1000);
        // m_shardDefinition->AddShard(2000);
        // m_shardDefinition->AddShard(3000);

        //m_ingestor = Factories::CreateIngestor(*m_fileManager,
        //                                       *m_schema,
        //                                       *m_recycler,
        //                                       *m_termTable,
        //                                       *m_shardDefinition,
        //                                       *m_sliceAllocator));

    }


    void SimpleIndex::StopIndex()
    {
        m_recycler->Shutdown();
    }


    IConfiguration const & SimpleIndex::GetConfiguration() const
    {
        return *m_configuration;
    }


    IRecycler & SimpleIndex::GetRecycler() const
    {
        return *m_recycler;
    }


    ITermTable2 const & SimpleIndex::GetTermTable() const
    {
        return *m_termTable;
    }
}
