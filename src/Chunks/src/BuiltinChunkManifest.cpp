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

#include "BitFunnel/Chunks/Factories.h"
#include "BitFunnel/Exceptions.h"
#include "BuiltinChunkManifest.h"
#include "ChunkIngestor.h"
#include "ChunkReader.h"


namespace BitFunnel
{
    std::unique_ptr<IChunkManifestIngestor>
        Factories::CreateBuiltinChunkManifest(
            BuiltinChunkManifest::ChunkArray const & chunks,
            IChunkProcessorFactory & factory)
    {
        return std::unique_ptr<IChunkManifestIngestor>(
            new BuiltinChunkManifest(chunks,
                                     factory));
    }


    BuiltinChunkManifest::BuiltinChunkManifest(ChunkArray const & chunks,
                                               IChunkProcessorFactory & factory)
      : m_chunks(chunks),
        m_factory(factory)
    {
    }


    size_t BuiltinChunkManifest::GetChunkCount() const
    {
        return m_chunks.size();
    }


    void BuiltinChunkManifest::IngestChunk(size_t index) const
    {
        if (index >= m_chunks.size())
        {
            FatalError error("ChunkManifestIngestor: chunk index out of range.");
            throw error;
        }

        auto processor = m_factory.Create();

        ChunkReader(m_chunks[index].second,
                    m_chunks[index].second + m_chunks[index].first,
                    *processor);

    }
}
