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

#include <fstream>
#include <iostream>
#include <sstream>

#include "BitFunnel/Chunks/Factories.h"
#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/Exceptions.h"
#include "BitFunnel/IFileManager.h"
#include "ChunkIngestor.h"
#include "ChunkManifestIngestor.h"
#include "ChunkReader.h"


namespace BitFunnel
{
    std::unique_ptr<IChunkManifestIngestor>
        Factories::CreateChunkManifestIngestor(
            IFileSystem& fileSystem,
            IChunkWriter * writer,
            std::vector<std::string> const & filePaths,
            IConfiguration const & config,
            IIngestor& ingestor,
            IDocumentFilter & filter,
            bool cacheDocuments)
    {
        return std::unique_ptr<IChunkManifestIngestor>(
            new ChunkManifestIngestor(
                fileSystem,
                writer,
                filePaths,
                config,
                ingestor,
                filter,
                cacheDocuments));
    }


    ChunkManifestIngestor::ChunkManifestIngestor(
        IFileSystem& fileSystem,
        IChunkWriter * writer,
        std::vector<std::string> const & filePaths,
        IConfiguration const & config,
        IIngestor& ingestor,
        IDocumentFilter & filter,
        bool cacheDocuments)
      : m_fileSystem(fileSystem),
        m_writer(writer),
        m_filePaths(filePaths),
        m_configuration(config),
        m_ingestor(ingestor),
        m_filter(filter),
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

        std::cout << "  " << m_filePaths[index] << std::endl;

        auto input = m_fileSystem.OpenForRead(m_filePaths[index].c_str(),
                                              std::ios::binary);

        if (input->fail())
        {
            std::stringstream message;
            message << "Failed to open chunk file '"
                << m_filePaths[index]
                << "'";
            throw FatalError(message.str());
        }

        input->seekg(0, input->end);
        auto length = input->tellg();
        input->seekg(0, input->beg);

        std::vector<char> chunkData;
        chunkData.reserve(static_cast<size_t>(length) + 1ull);
        chunkData.insert(chunkData.begin(),
                         (std::istreambuf_iterator<char>(*input)),
                          std::istreambuf_iterator<char>());

        {
            // Block scopes std::ostream.
            if (m_writer != nullptr)
            {
                m_writer->SetChunk(index);
            }

            ChunkIngestor processor(m_configuration,
                                    m_ingestor,
                                    m_cacheDocuments,
                                    m_filter,
                                    m_writer);

            ChunkReader(&chunkData[0],
                        &chunkData[0] + chunkData.size(),
                        processor);
        }
    }
}
