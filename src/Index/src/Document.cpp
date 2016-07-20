// The MIT License (MIT)

// Copyright (c) 2016, Microsoft

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "BitFunnel/Exceptions.h"
#include "Configuration.h"
#include "Document.h"


namespace BitFunnel
{
    Document::Document(IConfiguration const & config)
        : m_maxGramSize(config.GetMaxGramSize()),
          m_docFreqTable(config.GetDocumentFrequencyTable()),
          m_streamIsOpen(false)
    {
    }


    size_t Document::GetPostingCount() const
    {
        return m_postings.size();
    }


    void Document::Ingest(DocumentHandle handle) const
    {
        // TODO: convert unique terms into postings.
        for (auto it = m_postings.begin(); it != m_postings.end(); ++it)
        {
            handle.AddPosting(*it);
        }
    }


    void Document::OpenStream(Term::StreamId id)
    {
        if (m_streamIsOpen)
        {
            throw FatalError("Attempting OpenStream() when another stream is open.");
        }
        else
        {
            m_streamIsOpen = true;

            m_currentStreamId = id;

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
        LogAssertB(count > 0, "Count must be greater than 0.");

        // Process each of the higher order n-grams.
        Term term(m_ringBuffer[0]);
        AddPosting(term);
        for (size_t n = 1; n < count; ++n)
        {
            term.AddTerm(m_ringBuffer[n]);
            AddPosting(term);
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


    void Document::AddPosting(Term term)
    {
        m_postings.insert(term);
    }
}
