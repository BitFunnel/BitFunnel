#pragma once

#include "BitFunnel/Index/IDocument.h"      // Inherits from IDocument.
#include "BitFunnel/Utilities/RingBuffer.h" // RingBuffer member.
#include "Term.h"                           // Term template parameter.


namespace BitFunnel
{
    class IConfiguration;
    class IDocumentFrequencyTable;


    class Document : public IDocument
    {
    public:
        Document(IConfiguration const & config);

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
        // Invoke PostTerm() for each ngram starting at the front of
        // m_ringBuffer. This includes ngrams with lengths 1 to
        // IConfiguration::GetMaxGramSize.
        void ProcessNGrams();

        // Invoke PostTerm for each ngram starting at each position in
        // m_ringBuffer.
        void PurgeRingBuffer();

        // Add term to the set of terms used to create postings in a
        // call to Ingest().
        void PostTerm(Term term);

        //
        // Constructor parameters.
        //

        // Maximum size of ngrams that will be indexed.
        const size_t m_maxGramSize;

        // Not used. TODO: Remove this?
        IDocumentFrequencyTable const & m_docFreqTable;


        //
        // Other members.
        //

        // Temporary count of postings in this document.
        // TODO: replace with count from hash table.
        size_t m_postingCount;

        // Ring buffer used to generate ngram postings.
        static const size_t c_ringBufferSize = Term::c_log2MaxGramSize + 1;
        RingBuffer<Term, c_ringBufferSize> m_ringBuffer;


        //
        // State machine for OpenStream()/AddTerm()/CloseStream()
        //

        // True if OpenStream() has been called and CloseStream() has not
        // been called since OpenStream().
        bool m_streamIsOpen;

        // StreamId specified in last call to OpenStream.
        // Only valid when m_streamIsOpen is true.
        Term::StreamId m_currentStreamId;
    };
}
