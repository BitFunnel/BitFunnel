#pragma once

#include <stddef.h>     // size_t parameter.
#include <string>       // std::string template parameter.
#include <vector>       // std::vector member.

#include "BitFunnel/Utilities/ITaskProcessor.h"


namespace BitFunnel
{
    class IConfiguration;


    class ChunkTaskProcessor : public ITaskProcessor
    {
    public:
        ChunkTaskProcessor(std::vector<std::string> const & filePaths,
                           IConfiguration const & config,
                           IIngestor& ingestor);

        //
        // ITaskProcessor methods.
        //
        virtual void ProcessTask(size_t taskId) override;
        virtual void Finished() override;

    private:
        //
        // Constructor parameters.
        //
        std::vector<std::string> const & m_filePaths;
        IConfiguration const & m_config;
        IIngestor& m_ingestor;
    };
}