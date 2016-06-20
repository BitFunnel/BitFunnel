#pragma once

#include "BitFunnel/IInterface.h"   // Inherits from IInterface.


namespace BitFunnel
{
    class IDocumentFrequencyTable;


    //*************************************************************************
    //
    // IConfiguration is an abstract base class or interface for classes that
    // provide configuration data for the document ingestion pipeline.
    //
    //*************************************************************************
    class IConfiguration : public IInterface
    {
    public:
        // Returns the maximum ngram size that will be indexed.
        virtual size_t GetMaxGramSize() const = 0;

        // Returns an IDocumentFrequenyTable that is representative of the
        // distribution of terms in the corpus.
        virtual IDocumentFrequencyTable const & 
            GetDocumentFrequencyTable() const = 0;
    };
}
