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

#include <memory>   // std::unique_ptr return value.
#include <vector>   // std::vector parameter.
#include <stddef.h> // ::size_t parameter.
#include <string>   // std::string template parameter.

#include "BitFunnel/BitFunnelTypes.h"


namespace BitFunnel
{
    class IChunkManifestIngestor;
    class IChunkWriterFactory;
    class IConfiguration;
    class IDocument;
    class IDocumentFilter;
    class IFileManager;
    class IFileSystem;
    class IIngestor;
    class IShardDefinition;


    namespace Factories
    {
        std::unique_ptr<IChunkManifestIngestor>
            CreateBuiltinChunkManifest(
                std::vector<std::pair<size_t, char const *>> const & chunks,
                IConfiguration const & config,
                IIngestor& ingestor,
                bool cacheDocuments);


        std::unique_ptr<IChunkManifestIngestor>
            CreateChunkManifestIngestor(
                IFileSystem& fileSystem,
                IChunkWriterFactory* chunkWriterFactory,
                std::vector<std::string> const & filePaths,
                IConfiguration const & config,
                IIngestor& ingestor,
                IDocumentFilter & filter,
                bool cacheDocuments);


        std::unique_ptr<IChunkWriterFactory>
            CreateAnnotatingChunkWriterFactory(
                IFileManager& fileManager,
                IShardDefinition const & shardDefinition);

        std::unique_ptr<IChunkWriterFactory>
            CreateCopyingChunkWriterFactory(
                IFileManager& fileManager);

        std::unique_ptr<IDocument>
            CreateDocument(IConfiguration const & configuration, DocId id);
    }
}
