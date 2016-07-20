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

#pragma once

#include <unordered_set>                    // TODO: Remove this temporary include.

#include "BitFunnel/Index/IDocument.h"      // Inherits from IDocument.
#include "BitFunnel/Utilities/RingBuffer.h" // RingBuffer member.
#include "BitFunnel/Term.h"                 // Term template parameter.


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
        virtual void OpenStream(Term::StreamId id) override;

        // Adds a term to the currently opened stream.
        virtual void AddTerm(char const * term) override;

        // Closes the current stream.
        virtual void CloseStream() override;

    private:
        // Invoke AddPosting() for each ngram starting at the front of
        // m_ringBuffer. This includes ngrams with lengths 1 to
        // IConfiguration::GetMaxGramSize.
        void ProcessNGrams();

        // Invoke AddPosting() for each ngram starting at each position in
        // m_ringBuffer.
        void PurgeRingBuffer();

        // Add term to the set of terms used to create postings in a
        // call to Ingest().
        void AddPosting(Term term);

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

        // TODO: Replace unordered_set with alloc free version.
        std::unordered_set<Term, Term::Hasher> m_postings;
    };
}
