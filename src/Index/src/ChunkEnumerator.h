#pragma once

#include <memory>
#include <stddef.h>
#include <string>

#include "BitFunnel/NonCopyable.h"
#include "ChunkReader.h"
#include "IIndex.h"

namespace BitFunnel
{
    class IIndex;
    class ITaskDistributor;

    // Fill an std::vector with filenames
    // Construct a ChunkTaskProcessor for each thread
    // Pass the above to constructor TaskDistributor()
    class ChunkEnumerator : public NonCopyable
    {
    public:
        ChunkEnumerator(std::vector<std::string const> const & filePaths,
                        IIndex& index, size_t threadCount);
        void WaitForCompletion() const;

    private:
        IIndex& m_index;
        // std::unique_ptr<ITaskDistributor> m_distributor;
    };
}
