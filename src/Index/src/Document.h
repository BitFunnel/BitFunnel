#pragma once

#include "BitFunnel/Index/IDocument.h"  // Inherits from IDocument.


namespace BitFunnel
{
    class Document : public IDocument
    {
    public:
        Document();

        // Returns the number of postings this document will contribute
        // to the index. This method is used to determine which shard
        // will hold the document.
        virtual size_t GetPostingCount() const override;

        // Ingests the contents of this document into the index at via
        // the supplied DocumentHandle.
        virtual void Ingest(DocumentHandle handle) const override;


        // Opens a named stream for term additions. Subsequent calls to
        // AddTerm() will add terms to this stream.
        virtual void OpenStream(char const * name) override;

        // Adds a term to the currently opened stream.
        virtual void AddTerm(char const * term) override;

        // Closes the current stream. 
        virtual void CloseStream() override;

    private:
        size_t m_postingCount;
    };
}
