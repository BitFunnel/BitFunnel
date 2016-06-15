#pragma once

#include "BitFunnel/Index/DocumentHandle.h"     // DocumentHandle parameter.
#include "BitFunnel/IInterface.h"               // Inherits from IIndex.


namespace BitFunnel
{
    class IDocument : public IInterface
    {
    public:
        // Returns the number of postings this document will contribute
        // to the index. This method is used to determine which shard
        // will hold the document.
        virtual size_t GetPostingCount() const = 0;

        // Ingests the contents of this document into the index at via
        // the supplied DocumentHandle.
        virtual void Ingest(DocumentHandle handle) const = 0;
    };
}
