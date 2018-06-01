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


#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/Helpers.h"
#include "BitFunnel/Index/IRecycler.h"
#include "BitFunnel/Index/ISliceBufferAllocator.h"
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
          m_isStarted(false),
          m_blockAllocatorBufferSize(0)
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


    void SimpleIndex::SetBlockAllocatorBufferSize(size_t size)
    {
        m_blockAllocatorBufferSize = size;
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
            m_shardDefinition = Factories::LoadOrCreateDefaultShardDefinition(*m_fileManager);
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

        if (m_facts.get() == nullptr)
        {
            m_facts = Factories::CreateFactSet();
        }

        if (m_configuration.get() == nullptr)
        {
            m_configuration =
                Factories::CreateConfiguration(gramSize,
                                               generateTermToText,
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
            m_shardDefinition = Factories::LoadOrCreateDefaultShardDefinition(*m_fileManager);
        }

        if (m_termTables.get() == nullptr)
        {
            m_termTables =
                Factories::CreateTermTableCollection(
                    *m_fileManager,
                    m_shardDefinition->GetShardCount());
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
            const double defaultDensity = 0.15;
            m_shardDefinition->AddShard(0, defaultDensity);
        }

        if (m_termTables.get() == nullptr)
        {
            m_termTables =
                Factories::CreateTermTableCollection(
                    m_shardDefinition->GetShardCount());
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
                                               *m_facts);
        }
    }


    void SimpleIndex::StartIndex()
    {
        EnsureStarted(false);

        if (m_sliceAllocator.get() == nullptr)
        {
            // To calculate a large-enough m_blocksize, we need to calculate the
            // largest blocksize (slice) required by any TermTable (shard).
            size_t m_blockSize = 0;
            for (size_t tableId=0; tableId < m_termTables->size(); ++tableId)
            {
                const size_t tblBlockSize = GetReasonableBlockSize(*m_schema, m_termTables->GetTermTable(tableId));
                if (tblBlockSize > m_blockSize)
                {
                    m_blockSize = tblBlockSize;
                }
            }


            // If the user didn't specify the block allocator buffer size, use
            // a default that is small enough that it won't cause a CI failure.
            // The CI machines don't have a lot of memory, so it is necessary
            // to use a modest memory requirement (1GiB) to allow unit tests to pass.
            // See issue #388.
            if (m_blockAllocatorBufferSize == 0)
            {
                m_blockAllocatorBufferSize = 1073741824;
            }

            // Calculate number of slices that will fit in requested memory.
            // Ensure we have enough memory for at least one slice per termtable
            size_t blockCount = m_blockAllocatorBufferSize / m_blockSize;
            if (blockCount < m_termTables->size())
            {
                throw FatalError("Insufficient memory requested to build index");
            }

            m_sliceAllocator =
                Factories::CreateSliceBufferAllocator(m_blockSize,
                                                      blockCount);
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
