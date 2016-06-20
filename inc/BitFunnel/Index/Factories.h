#pragma once

#include <memory>

// TODO: Do we need to include headers for IConfiguration and IIngestor?
// It seems that std::unique_ptr<> needs them.

namespace BitFunnel
{
    class IConfiguration;
    class IIngestor;

    namespace Factories
    {
        std::unique_ptr<IConfiguration> CreateConfiguration(size_t maxGramSize);
        std::unique_ptr<IIngestor> CreateIngestor();
    }
}
