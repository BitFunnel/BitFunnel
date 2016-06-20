#pragma once

#include <memory>       // std::unique_ptr member.
#include <stddef.h>     // size_t parameter.
#include <string>       // std::string template parameter.

#include "BitFunnel/NonCopyable.h"                  // Inherits from NonCopyable.
#include "BitFunnel/Utilities/ITaskDistributor.h"   // std::unqiue_ptr template parameter.


namespace BitFunnel
{
    class IConfiguration;
    class IIngestor;

    // Fill an std::vector with filenames
    // Construct a ChunkTaskProcessor for each thread
    // Pass the above to constructor TaskDistributor()
    class ChunkEnumerator : public NonCopyable
    {
    public:
        ChunkEnumerator(std::vector<std::string> const & filePaths,
                        IConfiguration const & config,
                        IIngestor& ingestor,
                        size_t threadCount);

        void WaitForCompletion() const;

    private:
        std::unique_ptr<ITaskDistributor> m_distributor;
    };
}
