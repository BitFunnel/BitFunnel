
#include <iostream>

#include "BitFunnel/Index/IngestChunks.h"
#include "ChunkEnumerator.h"

namespace BitFunnel
{
    class IIngestor;

    void IngestChunks(std::vector<std::string> const & filePaths,
                      IIngestor& ingestor,
                      size_t threadCount)
    {
        ChunkEnumerator chunkEnumerator(filePaths, ingestor, threadCount);
        chunkEnumerator.WaitForCompletion();
    }
}
