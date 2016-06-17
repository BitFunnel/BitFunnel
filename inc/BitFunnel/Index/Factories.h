#pragma once

#include <memory>

namespace BitFunnel
{
    class IIngestor;

    namespace Factories
    {
        std::unique_ptr<IIngestor> CreateIngestor();
    }
}
