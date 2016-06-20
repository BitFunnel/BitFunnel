#include <iostream>     // TODO: Remove this temporary header.

#include "BitFunnel/Exceptions.h"
#include "Configuration.h"
#include "Document.h"


namespace BitFunnel
{
    Document::Document(IConfiguration const & config)
        : m_maxGramSize(config.GetMaxGramSize()),
          m_docFreqTable(config.GetDocumentFrequencyTable()),
          m_postingCount(0),
          m_streamIsOpen(false)
    {
    }


    size_t Document::GetPostingCount() const
    {
        return m_postingCount;
    }


    void Document::Ingest(DocumentHandle /*handle*/) const
    {
        // TODO: convert unique terms into postings.
        std::cout << "Document::Ingest()" << std::endl;
        std::cout << "  "
                  << m_terms.size() 
                  << " postings"
                  << std::endl;

        for (auto it = m_terms.begin(); it != m_terms.end(); ++it)
        {
            std::cout << "  ";
            (*it).Print(std::cout);
            std::cout << std::endl;
        }

        std::cout << "=====================" << std::endl;
    }


    void Document::OpenStream(char const * /*name*/)
    {
        if (m_streamIsOpen)
        {
            throw FatalError("Attempting OpenStream() when another stream is open.");
        }
        else
        {
            m_streamIsOpen = true;

            // TODO: set m_streamId correctly.
            m_currentStreamId = 0;

            // Reset ring buffer just in case.
            m_ringBuffer.Reset();
        }
    }


    void Document::AddTerm(char const * termText)
    {
        if (!m_streamIsOpen)
        {
            throw FatalError("Attempting AddTerm() with no open stream.");
        }
        else
        {
            // TODO: This is placeholder code that computes the term count.
            // TODO: Make it compute the unique posting count.
            ++m_postingCount;

            // TODO: should we use the dfThreshold parameter instead of the fixed value?
            new(m_ringBuffer.PushBack()) Term(termText,
                                              m_currentStreamId,
                                              m_docFreqTable);

            if (m_ringBuffer.GetCount() == m_maxGramSize)
            {
                ProcessNGrams();
                m_ringBuffer.PopFront();
            }
        }
    }


    void Document::CloseStream()
    {
        if (!m_streamIsOpen)
        {
            throw FatalError("Attempting CloseStream() with no open stream.");
        }
        else
        {
            m_streamIsOpen = false;

            // Process ngrams at end of document.
            PurgeRingBuffer();
        }
    }


    void Document::ProcessNGrams()
    {
        const size_t count = m_ringBuffer.GetCount();
        LogAssertB(count > 0);

        // Process each of the higher order n-grams.
        Term term(m_ringBuffer[0]);
        PostTerm(term);
        for (size_t n = 1; n < count; ++n)
        {
            term.AddTerm(m_ringBuffer[n]);
            PostTerm(term);
        }
    }


    void Document::PurgeRingBuffer()
    {
        while (!m_ringBuffer.IsEmpty())
        {
            ProcessNGrams();
            m_ringBuffer.PopFront();
        }
    }


    void Document::PostTerm(Term term)
    {
        term.Print(std::cout);
        std::cout << std::endl;
        m_terms.insert(term);
    }
}
