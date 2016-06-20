#include "BitFunnel/Index/IngestChunks.h"
#include "ChunkEnumerator.h"


namespace BitFunnel
{
    void IngestChunks(std::vector<std::string> const & filePaths,
                      IConfiguration const & config,
                      IIngestor& ingestor,
                      size_t threadCount)
    {
        ChunkEnumerator chunkEnumerator(filePaths, config, ingestor, threadCount);
        chunkEnumerator.WaitForCompletion();
    }
}
