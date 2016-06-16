#include "Document.h"


namespace BitFunnel
{
    Document::Document()
        : m_postingCount(0)
    {
    }


    size_t Document::GetPostingCount() const
    {
        return m_postingCount;
    }


    void Document::Ingest(DocumentHandle /*handle*/) const
    {
    }


    void Document::OpenStream(char const * /*name*/)
    {
    }


    void Document::AddTerm(char const * /*term*/)
    {
        // TODO: This is placeholder code that computes the term count.
        // TODO: Make it compute the unique posting count.
        ++m_postingCount;
    }


    void Document::CloseStream()
    {
    }
}
