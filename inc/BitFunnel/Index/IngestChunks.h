#pragma once

#include <vector>

namespace BitFunnel
{
    class IIngestor;

    void IngestChunks(std::vector<std::string> const & filePaths,
                      IIngestor& ingestor,
                      size_t threadCount);
}
