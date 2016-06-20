#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Index/Factories.h"
#include "Configuration.h"


namespace BitFunnel
{
    std::unique_ptr<IConfiguration>
        Factories::CreateConfiguration(size_t maxGramSize)
    {
        return std::unique_ptr<IConfiguration>(new Configuration(maxGramSize));
    }


    Configuration::Configuration(size_t maxGramSize)
        : m_maxGramSize(maxGramSize)
    {
    }


    size_t Configuration::GetMaxGramSize() const
    {
        return m_maxGramSize;
    }


    IDocumentFrequencyTable const &
        Configuration::GetDocumentFrequencyTable() const
    {
        // TODO: Implement this method.
        return *(IDocumentFrequencyTable*)nullptr;
    }
}
