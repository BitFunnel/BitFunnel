
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

        // NOTE: The act of constructing a ChunkIngestor causes the file to be
        // ingested.
        ChunkIngestor(readChunk(m_filePaths[taskId].c_str()), m_ingestor);
    }


    void ChunkTaskProcessor::Finished()
    {
    }
}
