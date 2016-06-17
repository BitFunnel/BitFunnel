
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "ChunkEnumerator.h"
#include "ChunkIngestor.h"
#include "ChunkTaskProcessor.h"

namespace BitFunnel
{
    ChunkTaskProcessor::ChunkTaskProcessor(
        std::vector<std::string> const & filePaths,
        IIngestor& ingestor)
      : m_filePaths(filePaths),
        m_ingestor(ingestor)
    {
    }


    void ChunkTaskProcessor::ProcessTask(size_t taskId)
    {
        std::cout << "ChunkTaskProcessor::ProcessTask: taskId:" << taskId
            << std::endl;
        std::cout << "ChunkTaskProcessor::ProcessTask: filePath:"
            << m_filePaths[taskId] << std::endl;

        std::ifstream inputStream(m_filePaths[taskId], std::ios::binary);
        std::vector<char> chunkData((std::istreambuf_iterator<char>(inputStream)),
                                    std::istreambuf_iterator<char>());

        // NOTE: The act of constructing a ChunkIngestor causes the bytes in
        // chunkData to be parsed into documents and ingested.
        ChunkIngestor(chunkData, m_ingestor);
    }


    void ChunkTaskProcessor::Finished()
    {
    }
}
