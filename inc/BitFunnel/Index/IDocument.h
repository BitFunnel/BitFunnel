#pragma once

#include "BitFunnel/Index/DocumentHandle.h"     // DocumentHandle parameter.
#include "BitFunnel/IInterface.h"               // Inherits from IInterface.


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


        // Opens a named stream for term additions. Subsequent calls to
        // AddTerm() will add terms to this stream.
        virtual void OpenStream(char const * name) = 0;

        // Adds a term to the currently opened stream.
        virtual void AddTerm(char const * term) = 0;

        // Closes the current stream. 
        virtual void CloseStream() = 0;
    };
}
