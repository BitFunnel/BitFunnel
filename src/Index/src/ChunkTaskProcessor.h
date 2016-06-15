#pragma once

#include <stddef.h>
#include <vector>

#include "BitFunnel/Utilities/ITaskProcessor.h"
#include "IIndex.h"

namespace BitFunnel
{
    class ChunkTaskProcessor : public ITaskProcessor
    {
    public:
        ChunkTaskProcessor(std::vector<std::string const> const & filePaths,
                           IIndex& index);

        //
        // ITaskProcessor methods.
        //
        virtual void ProcessTask(size_t taskId) override;
        virtual void Finished() override;

    private:
        std::vector<std::string const> const & m_filePaths;
        IIndex& m_index;
    };
}