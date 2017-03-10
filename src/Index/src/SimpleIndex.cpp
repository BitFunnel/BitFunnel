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
#include "BitFunnel/Index/Row.h"
#include "LoggerInterfaces/Check.h"
#include "SimpleIndex.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // Factory methods.
    //
    //*************************************************************************

    std::unique_ptr<ISimpleIndex>
        Factories::CreateSimpleIndex(IFileSystem& fileSystem)
    {
        return std::unique_ptr<ISimpleIndex>(new SimpleIndex(fileSystem));
    }


    //*************************************************************************
    //
    // SimpleIndex
    //
    //*************************************************************************

    SimpleIndex::SimpleIndex(IFileSystem& fileSystem)
        : m_fileSystem(fileSystem),
          m_isStarted(false)
    {
    }


    SimpleIndex::~SimpleIndex()
    {
        StopIndex();
    }


    //
    // Setter methods.
    //

    void SimpleIndex::SetConfiguration(
        std::unique_ptr<IConfiguration> config)
    {
        EnsureStarted(false);
        CHECK_EQ(m_configuration.get(), nullptr)
            << "Attempting to overwrite existing Configuration.";
        m_configuration = std::move(config);
    }


    void SimpleIndex::SetFactSet(
        std::unique_ptr<IFactSet> facts)
    {
        EnsureStarted(false);
        CHECK_EQ(m_facts.get(), nullptr)
            << "Attempting to overwrite existing FactSet.";
        m_facts = std::move(facts);
    }


    void SimpleIndex::SetFileManager(
        std::unique_ptr<IFileManager> fileManager)
    {
        EnsureStarted(false);
        CHECK_EQ(m_fileManager.get(), nullptr)
            << "Attempting to overwrite existing FileManager.";
        m_fileManager = std::move(fileManager);
    }


    //void SimpleIndex::SetFileSystem(
    //    std::unique_ptr<IFileSystem> fileSystem)
    //{
    //    EnsureStarted(false);
    //    CHECK_EQ(m_fileSystem.get(), nullptr)
    //        << "Attempting to overwrite existing FileSystem.";
    //    m_fileSystem = std::move(fileSystem);
    //}


    void SimpleIndex::SetIdfTable(
        std::unique_ptr<IIndexedIdfTable> idfTable)
    {
        EnsureStarted(false);
        CHECK_EQ(m_idfTable.get(), nullptr)
            << "Attempting to overwrite existing IndexIdfTable.";
        m_idfTable = std::move(idfTable);
    }


    void SimpleIndex::SetSchema(
        std::unique_ptr<IDocumentDataSchema> schema)
    {
        EnsureStarted(false);
        CHECK_EQ(m_schema.get(), nullptr)
            << "Attempting to overwrite existing DocumentDataSchema.";
        m_schema = std::move(schema);
    }


    void SimpleIndex::SetShardDefinition(
        std::unique_ptr<IShardDefinition> definition)
    {
        EnsureStarted(false);
        CHECK_EQ(m_shardDefinition.get(), nullptr)
            << "Attempting to overwrite existing ShardDefinition.";
        m_shardDefinition = std::move(definition);
    }


    void SimpleIndex::SetSliceBufferAllocator(
        std::unique_ptr<ISliceBufferAllocator> sliceAllocator)
    {
        EnsureStarted(false);
        CHECK_EQ(m_sliceAllocator.get(), nullptr)
            << "Attempting to overwrite existing ShardDefinition.";
        m_sliceAllocator = std::move(sliceAllocator);
    }


    void SimpleIndex::SetTermTableCollection(
        std::unique_ptr<ITermTableCollection> termTables)
    {
        EnsureStarted(false);
        CHECK_EQ(m_termTables.get(), nullptr)
            << "Attempting to overwrite existing TermTableCollection.";
        m_termTables = std::move(termTables);
    }


    //
    // Configuration methods.
    //

    void SimpleIndex::ConfigureForStatistics(char const * directory,
                                             size_t gramSize,
                                             bool generateTermToText)
    {
        EnsureStarted(false);

        //if (m_fileSystem.get() == nullptr)
        //{
        //    m_fileSystem = Factories::CreateFileSystem();
        //}

        if (m_fileManager.get() == nullptr)
        {
            m_fileManager = Factories::CreateFileManager(directory,
                                                         directory,
                                                         directory,
                                                         m_fileSystem);
        }

        // TODO: Load schema from file.
        if (m_schema.get() == nullptr)
        {
            m_schema = Factories::CreateDocumentDataSchema();
        }

        // TODO: consider making this work if no ShardDefinition exists.
        if (m_shardDefinition.get() == nullptr)
        {
            auto input = m_fileManager->ShardDefinition().OpenForRead();
            m_shardDefinition =
               Factories::CreateShardDefinition(*input);
        }

        if (m_termTables.get() == nullptr)
        {
            // When gathering corpus statistics, we don't yet have any
            // TermTables. For now just create a collection of default
            // initialized TermTables.
            m_termTables =
                Factories::CreateTermTableCollection(
                    m_shardDefinition->GetShardCount());
        }

        if (m_idfTable == nullptr)
        {
            // When we're building statistics we don't yet have an
            // IndexedIdfTable. Just use an empty one for now. This means
            // that terms that are created will all be marked with the
            // default IDF value. This is not a problem since the term
            // IDF values are not examined by the StatisticsBuild. They
            // exist primarily for the query pipeline where the TermTable
            // needs terms anotated with IDF values to handle the case
            // where the terms have an implicit, or adhoc mapping to
            // RowIds.
            m_idfTable = Factories::CreateIndexedIdfTable();
        }

        if (m_facts.get() == nullptr)
        {
            m_facts = Factories::CreateFactSet();
        }

        if (m_configuration.get() == nullptr)
        {
            m_configuration =
                Factories::CreateConfiguration(gramSize,
                                               generateTermToText,
                                               *m_idfTable,
                                               *m_facts);
        }
    }


    void SimpleIndex::ConfigureForServing(char const * directory,
                                          size_t gramSize,
                                          bool generateTermToText)
    {
        EnsureStarted(false);

        //if (m_fileSystem.get() == nullptr)
        //{
        //    m_fileSystem = Factories::CreateFileSystem();
        //}

        if (m_fileManager.get() == nullptr)
        {
            m_fileManager = Factories::CreateFileManager(directory,
                                                         directory,
                                                         directory,
                                                         m_fileSystem);
        }

        // TODO: Load schema from file.
        if (m_schema.get() == nullptr)
        {
            m_schema = Factories::CreateDocumentDataSchema();
        }

        // TODO: Load shard definition from file.
        if (m_shardDefinition.get() == nullptr)
        {
            m_shardDefinition =
                Factories::CreateShardDefinition();

            // The following shard-aware code causes problems when the
            // input file is missing. See issue 308.
            //      https://github.com/BitFunnel/BitFunnel/issues/308
            auto input = m_fileManager->ShardDefinition().OpenForRead();
            m_shardDefinition =
               Factories::CreateShardDefinition(*input);
        }

        if (m_termTables.get() == nullptr)
        {
            m_termTables =
                Factories::CreateTermTableCollection(
                    *m_fileManager,
                    m_shardDefinition->GetShardCount());
        }

        if (m_idfTable == nullptr)
        {
            auto input = m_fileManager->IndexedIdfTable(0).OpenForRead();
            Term::IdfX10 defaultIdf = 60;   // TODO: use proper value here.
            m_idfTable = Factories::CreateIndexedIdfTable(*input, defaultIdf);
        }

        if (m_facts.get() == nullptr)
        {
            m_facts = Factories::CreateFactSet();
        }

        if (m_configuration.get() == nullptr)
        {
            m_configuration =
                Factories::CreateConfiguration(gramSize,
                                               generateTermToText,
                                               *m_idfTable,
                                               *m_facts);
        }
    }


    void SimpleIndex::ConfigureAsMock(size_t gramSize,
                                      bool generateTermToText)
    {
        EnsureStarted(false);

        // TODO: Load schema from file.
        if (m_schema.get() == nullptr)
        {
            m_schema = Factories::CreateDocumentDataSchema();
        }

        // TODO: Load shard definition from file.
        if (m_shardDefinition.get() == nullptr)
        {
            m_shardDefinition =
                Factories::CreateShardDefinition();
        }

        if (m_termTables.get() == nullptr)
        {
            m_termTables =
                Factories::CreateTermTableCollection(
                    m_shardDefinition->GetShardCount());
        }

        if (m_idfTable == nullptr)
        {
            m_idfTable = Factories::CreateIndexedIdfTable();
        }

        if (m_facts.get() == nullptr)
        {
            m_facts = Factories::CreateFactSet();
        }

        if (m_configuration.get() == nullptr)
        {
            m_configuration =
                Factories::CreateConfiguration(gramSize,
                                               generateTermToText,
                                               *m_idfTable,
                                               *m_facts);
        }
    }


    void SimpleIndex::StartIndex()
    {
        EnsureStarted(false);

        if (m_sliceAllocator.get() == nullptr)
        {
            // TODO: Need a blockSize that works for all term tables.
            const ShardId tempId = 0;
            const size_t blockSize =
                32 * GetReasonableBlockSize(*m_schema, m_termTables->GetTermTable(tempId));
            //        std::cout << "Blocksize: " << blockSize << std::endl;

            const size_t initialBlockCount = 512;
            m_sliceAllocator =
                Factories::CreateSliceBufferAllocator(blockSize,
                                                      initialBlockCount);
        }

        if (m_recycler.get() == nullptr)
        {
            m_recycler = Factories::CreateRecycler();
            m_recyclerThread = std::thread(RecyclerThreadEntryPoint, this);
        }

        m_ingestor = Factories::CreateIngestor(*m_schema,
                                               *m_recycler,
                                               *m_termTables,
                                               *m_shardDefinition,
                                               *m_sliceAllocator);

        m_isStarted = true;
    }


    void SimpleIndex::StopIndex()
    {
        // StopIndex() can be called even if the index hasn't been started.
        // If SimpleIndex::SimpleIndex() throws, ~SimpleIndex() will be
        // invoked which will call StopIndex().
        // See issue 308.
        //    https://github.com/BitFunnel/BitFunnel/issues/308
        //EnsureStarted(true);

        if (m_recycler != nullptr)
        {
            m_recycler->Shutdown();
            m_recyclerThread.join();
        }
    }


    IConfiguration const & SimpleIndex::GetConfiguration() const
    {
        EnsureStarted(true);
        return *m_configuration;
    }


    IFileManager & SimpleIndex::GetFileManager() const
    {
        EnsureStarted(true);
        return *m_fileManager;
    }


    IFileSystem & SimpleIndex::GetFileSystem() const
    {
        EnsureStarted(true);
        return m_fileSystem;
    }


    IIngestor & SimpleIndex::GetIngestor() const
    {
        EnsureStarted(true);
        return *m_ingestor;
    }


    IRecycler & SimpleIndex::GetRecycler() const
    {
        EnsureStarted(true);
        return *m_recycler;
    }


    ITermTable const & SimpleIndex::GetTermTable0() const
    {
        return GetTermTable(0);
    }


    ITermTable const & SimpleIndex::GetTermTable(ShardId shardId) const
    {
        EnsureStarted(true);

        return m_termTables->GetTermTable(shardId);
    }


    void SimpleIndex::EnsureStarted(bool started) const
    {
        CHECK_EQ(started, m_isStarted)
            << (started ? "not allowed before starting index." :
                "not allowed after starting index.");

    }


    void SimpleIndex::RecyclerThreadEntryPoint(void * data)
    {
        SimpleIndex* index = reinterpret_cast<SimpleIndex*>(data);
        index->m_recycler->Run();
    }
}
