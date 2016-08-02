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
#include <sstream>

#include "BitFunnel/Exceptions.h"
#include "ChunkIngestor.h"
#include "ChunkTaskProcessor.h"


namespace BitFunnel
{
    ChunkTaskProcessor::ChunkTaskProcessor(
        std::vector<std::string> const & filePaths,
        IConfiguration const & config,
        IIngestor& ingestor)
      : m_filePaths(filePaths),
        m_config(config),
        m_ingestor(ingestor)
    {
    }


    void ChunkTaskProcessor::ProcessTask(size_t taskId)
    {
        if (taskId >= m_filePaths.size())
        {
            std::stringstream message;
            message << "No task corresponds to task id '" << taskId << "'";
            throw FatalError(message.str());
        }

        // // TODO: Replace stream to cout with calls to the logger.
        // std::cout << "ChunkTaskProcessor::ProcessTask: taskId:" << taskId
        //           << std::endl;
        // std::cout << "ChunkTaskProcessor::ProcessTask: filePath:"
        //           << m_filePaths[taskId] << std::endl;

        std::ifstream inputStream(m_filePaths[taskId], std::ios::binary);
        if (!inputStream.is_open())
        {
            std::stringstream message;
            message << "Failed to open chunk file '"
                    << m_filePaths[taskId]
                    << "'";
            throw FatalError(message.str());
        }

        std::vector<char> chunkData(
            (std::istreambuf_iterator<char>(inputStream)),
            std::istreambuf_iterator<char>());

        // NOTE: The act of constructing a ChunkIngestor causes the bytes in
        // chunkData to be parsed into documents and ingested.
        ChunkIngestor(chunkData, m_config, m_ingestor);
    }


    void ChunkTaskProcessor::Finished()
    {
    }
}
