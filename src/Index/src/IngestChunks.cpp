
#include <iostream>

#include "BitFunnel/Index/IngestChunks.h"
#include "ChunkEnumerator.h"

namespace BitFunnel
{
    void IngestChunks(std::vector<std::string> const & filePaths,
                      IIndex& index, size_t threadCount)
    {
        ChunkEnumerator chunkEnumerator(filePaths, index, threadCount);
        chunkEnumerator.WaitForCompletion();
    }
}
