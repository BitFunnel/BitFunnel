
#include <iostream>

#include "ChunkEnumerator.h"
#include "ChunkIngestor.h"
#include "ChunkTaskProcessor.h"

namespace BitFunnel
{
    ChunkTaskProcessor::ChunkTaskProcessor(
        std::vector<std::string> const & filePaths,
        IIndex& index)
      : m_filePaths(filePaths),
        m_index(index)
    {
    }

    void ChunkTaskProcessor::ProcessTask(size_t taskId)
    {
        std::cout << "ChunkTaskProcessor::ProcessTask: taskId:" << taskId
            << std::endl;
        std::cout << "ChunkTaskProcessor::ProcessTask: filePath:"
            << m_filePaths[taskId] << std::endl;
        m_index.noop();

        // NOTE: The act of constructing a ChunkIngestor causes the file to be
        // ingested.
        ChunkIngestor(m_filePaths[taskId], m_index);
    }

    void ChunkTaskProcessor::Finished()
    {
    }
}
