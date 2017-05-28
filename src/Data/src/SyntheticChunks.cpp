// The MIT License (MIT)
//
// Copyright (c) 2016, Microsoft
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <iomanip>
#include <iostream>     // TODO: removed this debugging code.
#include <limits>
#include <sstream>

#include "BitFunnel/Data/SyntheticChunks.h"
#include "BitFunnel/Configuration/IShardDefinition.h"
#include "LoggerInterfaces/Check.h"


namespace BitFunnel
{
    const size_t SyntheticChunks::m_shardDocIdStart;

    SyntheticChunks::SyntheticChunks(IShardDefinition const & shardDefinition,
                                     size_t documentsPerShard,
                                     size_t chunkCount)
        : m_documentsPerShard(documentsPerShard),
          m_chunkCount(chunkCount)
    {
        CHECK_GT(m_shardDocIdStart, documentsPerShard)
            << "documentsPerShard exceeds for shard DocId range.";

        // Determine scale factor.
        // Find shard with smallest number of legal length values.
        size_t minRange = std::numeric_limits<size_t>::max();
        size_t previousEnd = 0;

        for (ShardId shard = 0; shard < shardDefinition.GetShardCount() - 1; ++shard)
        {
            m_starts.push_back(previousEnd);
            m_ends.push_back(shardDefinition.GetMaxPostingCount(shard));

            size_t range = m_ends.back() - m_starts.back();
            std::cout << "Shard " << shard << ": range = " << range << std::endl;
            if (range < minRange)
            {
                minRange = range;
            }

            previousEnd = m_ends.back();
        }
        m_starts.push_back(previousEnd);
        m_ends.push_back(std::numeric_limits<size_t>::max());

        m_scale = (documentsPerShard + minRange - 1) / minRange;
        std::cout << "Scale: " << m_scale << std::endl;
    }


    size_t SyntheticChunks::GetChunkCount() const
    {
        return m_chunkCount;
    }


    void SyntheticChunks::WriteChunk(std::ostream& out, size_t chunkId) const
    {
        std::cout << std::endl << "Chunk " << chunkId << std::endl;
        for (size_t shard = 0; shard < m_starts.size(); ++shard)
        {
            size_t count = 0;
            size_t start = m_starts[shard] * m_scale;

            std::cout << std::endl << "  Shard " << shard << std::endl;

            for (size_t i = 0; i < m_documentsPerShard; ++i)
            {
                size_t docId = start + i;
                if (docId % m_chunkCount == chunkId)
                {
                    std::cout << "    Doc " << docId + shard * m_shardDocIdStart << ": [0.." << docId / m_scale << "]" << std::endl;

                    WriteDocument(out, docId + shard * m_shardDocIdStart);

                    ++count;
                    if (count == m_documentsPerShard)
                    {
                        break;
                    }
                }
            }
        }
        // End chunk.
        out << static_cast<char>(0);

    }


    void SyntheticChunks::WriteDocument(std::ostream& out, DocId docId) const
    {
        out << std::setfill('0') << std::setw(16) << std::hex << docId << static_cast<char>(0) << std::dec;
        out << "00" << static_cast<char>(0);

        // size_t end = docId / m_scale;
        size_t end = (docId % m_shardDocIdStart) / m_scale;
        for (size_t i = 0; i <= end; ++i)
        {
            out << i << static_cast<char>(0);
        }

        // End stream
        out << static_cast<char>(0);

        // End document
        out << static_cast<char>(0);
    }


    std::string SyntheticChunks::GetChunkName(size_t chunkId) const
    {
        std::stringstream name;
        name << "chunk-" << chunkId;
        return name.str();
    }


    //const_iterator SyntheticChunks::DocIds(ShardId shard) const;
    //const_iterator SyntheticChunks::end() const;

    bool SyntheticChunks::Contains(DocId docId, size_t value) const
    {
        size_t end = (docId % 100000) / m_scale;
        return value <= end;
    }
}
