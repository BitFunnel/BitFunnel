#pragma once

#include <stddef.h>
#include <vector>

#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Utilities/ITaskProcessor.h"

namespace BitFunnel
{
    class ChunkTaskProcessor : public ITaskProcessor
    {
    public:
        ChunkTaskProcessor(std::vector<std::string> const & filePaths,
                           IIngestor& ingestor);

        //
        // ITaskProcessor methods.
        //
        virtual void ProcessTask(size_t taskId) override;
        virtual void Finished() override;

    private:
        std::vector<std::string> const & m_filePaths;
        IIngestor& m_ingestor;
    };
}