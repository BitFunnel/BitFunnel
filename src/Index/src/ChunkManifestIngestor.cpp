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
#include <fstream>
#include <sstream>

#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Index/Factories.h"
#include "ChunkIngestor.h"
#include "ChunkManifestIngestor.h"


namespace BitFunnel
{
    std::unique_ptr<IChunkManifestIngestor>
        Factories::CreateChunkManifestIngestor(
            std::vector<std::string> const & filePaths,
            IConfiguration const & config,
            IIngestor& ingestor,
            bool cacheDocuments)
    {
        return std::unique_ptr<IChunkManifestIngestor>(
            new ChunkManifestIngestor(filePaths,
                                      config,
                                      ingestor,
                                      cacheDocuments));
    }


    ChunkManifestIngestor::ChunkManifestIngestor(std::vector<std::string> const & filePaths,
                                                 IConfiguration const & config,
                                                 IIngestor& ingestor,
                                                 bool cacheDocuments)
      : m_filePaths(filePaths),
        m_config(config),
        m_ingestor(ingestor),
        m_cacheDocuments(cacheDocuments)
    {
    }


    size_t ChunkManifestIngestor::GetChunkCount() const
    {
        return m_filePaths.size();
    }


    void ChunkManifestIngestor::IngestChunk(size_t index) const
    {
        if (index >= m_filePaths.size())
        {
            FatalError error("ChunkManifestIngestor: chunk index out of range.");
            throw error;
        }

        std::cout
            << "ChunkManifestIngestor::IngestChunk: filePath = "
            << m_filePaths[index] << std::endl;

        std::ifstream input(m_filePaths[index], std::ios::binary);
        if (!input.is_open())
        {
            std::stringstream message;
            message << "Failed to open chunk file '"
                << m_filePaths[index]
                << "'";
            throw FatalError(message.str());
        }

        input.seekg(0, input.end);
        auto length = input.tellg();
        input.seekg(0, input.beg);

        std::vector<char> chunkData;
        chunkData.reserve(static_cast<size_t>(length) + 1ull);
        chunkData.insert(chunkData.begin(),
                         (std::istreambuf_iterator<char>(input)),
                         std::istreambuf_iterator<char>());

        // NOTE: The act of constructing a ChunkIngestor causes the bytes in
        // chunkData to be parsed into documents and ingested.
        ChunkIngestor(&chunkData[0],
                      &chunkData[0] + chunkData.size(),
                      m_config,
                      m_ingestor,
                      m_cacheDocuments);
    }
}
