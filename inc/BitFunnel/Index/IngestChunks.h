#pragma once

#include <vector>

namespace BitFunnel
{
    class IConfiguration;
    class IIngestor;

    void IngestChunks(std::vector<std::string> const & filePaths,
                      IConfiguration const & config,
                      IIngestor& ingestor,
                      size_t threadCount);
}
