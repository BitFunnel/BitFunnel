
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "ChunkEnumerator.h"
#include "ChunkIngestor.h"
#include "ChunkTaskProcessor.h"

namespace BitFunnel
{
    static std::vector<char> readChunk(char const * fileName)
    {
        // TODO: Make this RAII.
        std::ifstream file(fileName);

        if (!file.is_open()) {
            // TODO: Make this a sensible error.
            throw 99;
        }

        std::stringstream ss;
        std::string line;
        while(std::getline(file, line)) {
            ss << line;
        }

        file.close();

        std::string lines = ss.str();
        const std::vector<char> chunkData = std::vector<char>(
            lines.c_str(),
            lines.c_str() + lines.size());

        return chunkData;
    }

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
        ChunkIngestor(readChunk(m_filePaths[taskId].c_str()), m_index);
    }

    void ChunkTaskProcessor::Finished()
    {
    }
}
