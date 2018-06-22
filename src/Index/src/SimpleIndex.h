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

#pragma once

#include <memory>                                   // std::unique_ptr embedded.
#include <thread>                                   // std::thread embedded.

#include "BitFunnel/Configuration/IFileSystem.h"    // Parameterizes std::unique_ptr.
#include "BitFunnel/Configuration/IShardDefinition.h"  // Parameterizes std::unique_ptr.
#include "BitFunnel/IFileManager.h"                 // Parameterizes std::unique_ptr.
#include "BitFunnel/Index/IConfiguration.h"         // Parameterizes std::unique_ptr.
#include "BitFunnel/Index/IDocumentDataSchema.h"    // Parameterizes std::unique_ptr.
#include "BitFunnel/Index/IIngestor.h"              // Parameterizes std::unique_ptr.
#include "BitFunnel/Index/IRecycler.h"              // Parameterizes std::unique_ptr.
#include "BitFunnel/Index/ISliceBufferAllocator.h"  // Parameterizes std::unique_ptr.
#include "BitFunnel/Index/ISimpleIndex.h"           // Parameterizes std::unique_ptr.
#include "BitFunnel/Index/ITermTable.h"             // Parameterizes std::unique_ptr.
#include "BitFunnel/Index/ITermTableCollection.h"   // Parameterizes std::unique_ptr.
#include "BitFunnel/NonCopyable.h"                  // Base class.
#include "BitFunnel/Term.h"                         // Term::GramSize embedded.


namespace BitFunnel
{
    class SimpleIndex : public ISimpleIndex, public NonCopyable
    {
    public:
        SimpleIndex(IFileSystem& fileSystem);

        virtual ~SimpleIndex();

        //
        // Configuration override methods.
        // Call these methods before StartIndex() to override the
        // default behavior.
        //
        virtual void SetConfiguration(
            std::unique_ptr<IConfiguration> config) override;
        virtual void SetFactSet(
            std::unique_ptr<IFactSet> facts) override;
        virtual void SetFileManager(
            std::unique_ptr<IFileManager> fileManager) override;
        //virtual void SetFileSystem(
        //    std::unique_ptr<IFileSystem> fileSystem) override;
        virtual void SetSchema(
            std::unique_ptr<IDocumentDataSchema> schema) override;
        virtual void SetShardDefinition(
            std::unique_ptr<IShardDefinition> definition) override;

        virtual void SetBlockAllocatorBufferSize(size_t size) override;
        virtual void SetSliceBufferAllocator(
            std::unique_ptr<ISliceBufferAllocator> sliceAllocator) override;

        virtual void SetTermTableCollection(
            std::unique_ptr<ITermTableCollection> termTables) override;


        virtual void ConfigureForStatistics(char const * directory,
                                            size_t gramSize,
                                            bool generateTermToText) override;

        virtual void ConfigureForServing(char const * directory,
                                         size_t gramSize,
                                         bool generateTermToText) override;

        virtual void ConfigureAsMock(size_t gramSize,
                                     bool generateTermToText) override;


        virtual void StartIndex() override;
        virtual void StopIndex() override;

        virtual IConfiguration const & GetConfiguration() const override;
        virtual IFileManager & GetFileManager() const override;
        virtual IFileSystem & GetFileSystem() const override;
        virtual IIngestor & GetIngestor() const override;
        virtual IRecycler & GetRecycler() const override;
        virtual ITermTable const & GetTermTable(ShardId shardId) const override;

    private:
        void EnsureStarted(bool started) const;
        void ConfigureSliceAllocator();

        static void RecyclerThreadEntryPoint(void * data);

        //
        // Constructor parameters.
        //

        IFileSystem& m_fileSystem;

        bool m_isStarted;

        //
        // Members initialized by StartIndex().
        //

//        std::unique_ptr<IFileSystem> m_fileSystem;
        std::unique_ptr<IFactSet> m_facts;
        std::unique_ptr<IFileManager> m_fileManager;
        std::unique_ptr<IDocumentDataSchema> m_schema;
        std::unique_ptr<IRecycler> m_recycler;
        std::thread m_recyclerThread;

        // Following members may become per-shard.
        std::unique_ptr<ITermTableCollection> m_termTables;
        std::unique_ptr<IConfiguration> m_configuration;

        size_t m_blockAllocatorBufferSize;
        std::unique_ptr<ISliceBufferAllocator> m_sliceAllocator;
        std::unique_ptr<IShardDefinition> m_shardDefinition;

        std::unique_ptr<IIngestor> m_ingestor;
    };
}
