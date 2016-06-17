#pragma once

#include <memory>
#include <stddef.h>
#include <string>

#include "BitFunnel/Index/IIndex.h"
#include "BitFunnel/NonCopyable.h"
#include "BitFunnel/Utilities/ITaskDistributor.h"
#include "ChunkReader.h"


namespace BitFunnel
{
    class IIngestor;
    class ITaskDistributor;

    // Fill an std::vector with filenames
    // Construct a ChunkTaskProcessor for each thread
    // Pass the above to constructor TaskDistributor()
    class ChunkEnumerator : public NonCopyable
    {
    public:
        ChunkEnumerator(std::vector<std::string> const & filePaths,
                        IIngestor& ingestor,
                        size_t threadCount);

        void WaitForCompletion() const;

    private:
        IIngestor& m_ingestor;
        std::unique_ptr<ITaskDistributor> m_distributor;
    };
}
