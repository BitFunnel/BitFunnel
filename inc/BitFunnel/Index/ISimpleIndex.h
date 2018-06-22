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

#include <memory>                       // std::unique_ptr parameter.

#include "BitFunnel/BitFunnelTypes.h"   // ShardId parameter.
#include "BitFunnel/IInterface.h"       // Base class.


namespace BitFunnel
{
    class IConfiguration;
    class IDocumentDataSchema;
    class IFactSet;
    class IFileManager;
    class IFileSystem;
    class IIngestor;
    class IRecycler;
    class IShardDefinition;
    class ISliceBufferAllocator;
    class ITermTable;
    class ITermTableCollection;


    //*************************************************************************
    //
    // ISimpleIndex
    //
    // An abstract base class or interface for convenience classes that
    // instantiate and wire up all of the classes needed to form a BitFunnel
    // Index.
	//
	// The intended usage pattern is to instantiate a class that implements
	// ISimpleIndex, then use setter methods to override default configuration
	// of various index components. Then call StartIndex() which will supply
	// default configuration components that weren't provider by via a setter.
    //
    //*************************************************************************
    class ISimpleIndex : public IInterface
    {
    public:
        virtual void SetConfiguration(
            std::unique_ptr<IConfiguration> config) = 0;
        virtual void SetFactSet(
            std::unique_ptr<IFactSet> facts) = 0;
        virtual void SetFileManager(
            std::unique_ptr<IFileManager> fileManager) = 0;
        //virtual void SetFileSystem(
        //    std::unique_ptr<IFileSystem> fileSystem) = 0;
        virtual void SetSchema(
            std::unique_ptr<IDocumentDataSchema> schema) = 0;
        virtual void SetShardDefinition(
            std::unique_ptr<IShardDefinition> definition) = 0;

        //
        // There are three options for the BlockAllocator:
        //   1. Provide an ISliceBufferAllocator&.
        //   2. Specify the amount of memory to use for Slice buffers and let
        //      StartIndex() instantiate its own ISliceBufferAllocator.
        //   3. Let StartIndex() choose sensible default values that ensure that unit
        //      tests can run under continuous integration with limited memory.
        //
        virtual void SetBlockAllocatorBufferSize(size_t size) = 0;
        virtual void SetSliceBufferAllocator(
            std::unique_ptr<ISliceBufferAllocator> sliceAllocator) = 0;

        virtual void SetTermTableCollection(
            std::unique_ptr<ITermTableCollection> termTables) = 0;

        virtual void ConfigureForStatistics(char const * directory,
                                            size_t gramSize,
                                            bool generateTermToText) = 0;

        virtual void ConfigureForServing(char const * directory,
                                         size_t gramSize,
                                         bool generateTermToText) = 0;

        virtual void ConfigureAsMock(size_t gramSize,
                                     bool generateTermToText) = 0;

        // Instantiates all of the classes necessary to form a BitFunnel Index.
        // Then starts the index. If forStatistics == true, the index will be
        // started for statistics generation, gathering data for
        //      Document Frequency Table
        //      Cumulative Term Counts
        //      Document Length Histogram
        //      Indexed Idf Table
        // If forStatistics == false, the index will be started for document
        // ingestion and query processing. In this case, it will read files
        // like
        //      Term Table
        //      Indexed Idf Table
        //
        // Note: this method starts a background thread for the IRecycler.
        // This thread is shut down in StopIndex().
        virtual void StartIndex() = 0;

        // Performs an orderly shutdown, then tears down all of the classes
        // created by StartIndex(). Must be called before class destruction.
        virtual void StopIndex() = 0;

        virtual IConfiguration const & GetConfiguration() const = 0;
        virtual IFileManager & GetFileManager() const = 0;
        virtual IFileSystem & GetFileSystem() const = 0;
        virtual IIngestor & GetIngestor() const = 0;
        virtual IRecycler & GetRecycler() const = 0;

        virtual ITermTable const & GetTermTable(ShardId shardId) const = 0;
    };
}
