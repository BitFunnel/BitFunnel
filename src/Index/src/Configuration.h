#pragma once

#include "BitFunnel/Index/IConfiguration.h"     // Inherits from IConfiguration.


namespace BitFunnel
{
    class IDocumentFrequencyTable;


    class Configuration : public IConfiguration
    {
    public:
        Configuration(size_t maxGramSize);

        virtual size_t GetMaxGramSize() const override;

        virtual IDocumentFrequencyTable const & 
            GetDocumentFrequencyTable() const override;

    private:
        size_t m_maxGramSize;
    };
}
