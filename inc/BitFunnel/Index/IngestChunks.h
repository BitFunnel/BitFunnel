#pragma once

#include <memory>
#include <vector>

#include "IIndex.h"

namespace BitFunnel
{
    void IngestChunks(std::vector<std::string> const & filePaths,
                      IIndex& index, size_t threadCount);
}