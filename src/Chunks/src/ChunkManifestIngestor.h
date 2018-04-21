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

#include <vector>   // std::vector parameter.
#include <string>   // Template parameter.

#include "BitFunnel/Chunks/IChunkManifestIngestor.h"   // Base class.


namespace BitFunnel
{
    class IConfiguration;
    class IChunkWriterFactory;
    class IDocumentFilter;
    class IFileSystem;
    class IIngestor;


    //*************************************************************************
    //
    // ChunkManifestIngestor
    //
    // Uses the BitFunnel ingestion pipeline to process a set of chunk/corpus
    // files into a stream of IDocuments, which are then be filtered and then
    // either written to disk or ingested into the index.
    //
    //*************************************************************************
    class ChunkManifestIngestor : public IChunkManifestIngestor
    {
    public:
        // Constructor parameters:
        //   fileSystem: the filesystem that hosts the input chunks.
        //
        //   chunkWriterFactory:
        //     If supplied, this factory's writers will be used be used to
        //     write documents that make it through the filter. If nullptr
        //     is passed, the documents will be ingested into the
        //     index. Writers have the opportunity to modify or annotate
        //     the document before writing. One use case is adding pseudo
        //     terms that indicate which shard holds a document.
        //
        //   filePaths:
        //      A vector of paths to input chunk files.
        //
        //   config:
        //      The IConfiguration that specifies ingestion parameters
        //      (e.g. maximum n-gram size).
        //
        //   ingestor:
        //      The IIngestor for the current index.
        //
        //   filter:
        //      An IDocumentFilter that determines which documents
        //      will be processed (copied or ingested).
        //
        //   cacheDocuments:
        //      If true, ingested documents will also be stored in a
        //      cache in order to allow for query verification with
        //      the `verify one` and `verify log` commands in the
        //      BitFunnel repl.
        //
        ChunkManifestIngestor(IFileSystem & fileSystem,
                              IChunkWriterFactory* chunkWriterFactory,
                              std::vector<std::string> const & filePaths,
                              IConfiguration const & config,
                              IIngestor & ingestor,
                              IDocumentFilter & filter,
                              bool cacheDocuments);

        //
        // IChunkManifestIngestor methods
        //

        virtual size_t GetChunkCount() const override;

        virtual void IngestChunk(size_t index) const override;

    private:

        //
        // Constructor parameters
        //

        IFileSystem & m_fileSystem;
        IChunkWriterFactory * m_chunkWriterFactory;
        std::vector<std::string> const & m_filePaths;
        IConfiguration const & m_configuration;
        IIngestor& m_ingestor;
        IDocumentFilter & m_filter;
        bool m_cacheDocuments;
    };
}
