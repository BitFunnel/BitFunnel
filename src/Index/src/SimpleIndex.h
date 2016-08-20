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
#include <thread>                                    // std::thread embedded.

#include "BitFunnel/Configuration/IShardDefinition.h"  // Parameterizes std::unique_ptr.
#include "BitFunnel/IFileManager.h"                 // Parameterizes std::unique_ptr.
#include "BitFunnel/Index/IConfiguration.h"         // Parameterizes std::unique_ptr.
#include "BitFunnel/Index/IDocumentDataSchema.h"    // Parameterizes std::unique_ptr.
#include "BitFunnel/Index/IIndexedIdfTable.h"       // Parameterizes std::unique_ptr.
#include "BitFunnel/Index/IIngestor.h"              // Parameterizes std::unique_ptr.
#include "BitFunnel/Index/IRecycler.h"              // Parameterizes std::unique_ptr.
#include "BitFunnel/Index/ISliceBufferAllocator.h"  // Parameterizes std::unique_ptr.
#include "BitFunnel/Index/ISimpleIndex.h"           // Parameterizes std::unique_ptr.
#include "BitFunnel/Index/ITermTableCollection.h"   // Parameterizes std::unique_ptr.
#include "BitFunnel/Noncopyable.h"                  // Base class.
#include "BitFunnel/Term.h"                         // Term::GramSize embedded.


namespace BitFunnel
{
    class SimpleIndex : public ISimpleIndex, public NonCopyable
    {
    public:
        SimpleIndex(char const * directory,
                    size_t gramSize,
                    bool generateTermtoText);

        virtual void StartIndex(bool forStatistics) override;
        virtual void StopIndex() override;

        virtual IConfiguration const & GetConfiguration() const override;
        virtual IFileManager & GetFileManager() const override;
        virtual IIngestor & GetIngestor() const override;
        virtual IRecycler & GetRecycler() const override;
        virtual ITermTable2 const & GetTermTable() const override;


    private:
        static void RecyclerThreadEntryPoint(void * data);

        //
        // Constructor parameters.
        //

        std::string m_directory;
        Term::GramSize m_gramSize;
        bool m_generateTermToText;


        //
        // Members initialized by StartIndex().
        //

        std::unique_ptr<IFileManager> m_fileManager;
        std::unique_ptr<IDocumentDataSchema> m_schema;
        std::unique_ptr<IRecycler> m_recycler;
        std::thread m_recyclerThread;

        // Following members may become per-shard.
        std::unique_ptr<ITermTableCollection> m_termTables;
        std::unique_ptr<IIndexedIdfTable> m_idfTable;
        std::unique_ptr<IConfiguration> m_configuration;

        std::unique_ptr<ISliceBufferAllocator> m_sliceAllocator;
        std::unique_ptr<IShardDefinition> m_shardDefinition;

        std::unique_ptr<IIngestor> m_ingestor;
    };
}
