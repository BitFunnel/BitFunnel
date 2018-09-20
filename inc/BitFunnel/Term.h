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

#include <iosfwd>                       // std::ostream parameter.
#include <limits>
#include <stdint.h>                     // uint8_t, uint64_t members.

#include "BitFunnel/BitFunnelTypes.h"   // Rank parameter.


namespace BitFunnel
{
    class IConfiguration;
    class NGramBuilder;
    class IObjectParser;

    //*************************************************************************
    //
    // Class Term
    //
    // A term represents a word or phrase from a document along with a
    // classification that specifies how the term should be used in matching
    // and scoring. The term stores a 64-bit hash of its text instead of the
    // actual text. Since the text is not retained, phrases cannot be
    // inspected to determine n-gram size. For this reason, terms explicitly
    // store their n-gram sizes.
    //
    // IMPORTANT NOTE: In order for the index to work correctly, it is
    // essential  that the IIndexedDocFreqTable supplied at index build time
    // has the same behavior as the one supplied at index serve time.
    //*************************************************************************
    class Term
    {
    public:
        // Terms are ngrams (n-word phrases). GramSize is the number of words
        // in the phrase.
        typedef uint8_t GramSize;

        // Hash of the term's text.
        typedef uint64_t Hash;

        // Idf (indexed document frequency) is defined as the log base-10 of
        // the total document count divided by how many documents contain the term:
        //     Idf = log(1/term-frequency)
        // For example, a term that appears once in every 1000 documents will
        // have an IDF of 3. For more info, see:
        //      http://en.wikipedia.org/wiki/Inverse_document_frequency
        // IdfX10 is a one-byte unsigned integer, equal to the actual IDF value
        // multiplied by 10 and rounded to the nearest integer.
        typedef uint8_t IdfX10;

        // Terms come from various streams in documents (e.g. title stream,
        // body stream, etc.).
        typedef uint8_t StreamId;


        // Number of bits required to represent a GramSize.
        static const GramSize c_log2MaxGramSize = 3;
        static const GramSize c_maxGramSize = (1 << c_log2MaxGramSize) - 1;
        static const IdfX10 c_maxIdfX10Value = 60;
        // TODO: what should this value really be?
        //static const IdfX10 c_maxIdfSumX10Value = std::numeric_limits<IdfX10>::max() - 1;
        static const IdfX10 c_maxIdfSumX10Value = (std::numeric_limits<IdfX10>::max)() - 1;

        // TODO: Should terms store ngram sizes? Why?

        // Constructs a Term for a unigram based on its raw hash
        // and classification. Its document frequency provides the term's
        // IdfSum value. The IMetaWordTierHintMap is used to set the term's
        // TierHint value.
        Term(char const * text,
             StreamId stream,
             IConfiguration const & configuration);

        // Constructs a term from its components. Common use case it to
        // construct a unigram (GramSize == 1). GramSize parameter exists
        // for TermTableBuilder scenario where adhoc term prototypes are
        // enumerated for idf in (0..c_maxIdfValue) and gramSize in
        // (1..c_maxGramSize).
        Term(Hash rawHash,
             StreamId stream,
             GramSize = 1);

        Term(IObjectParser& parser, bool parseParametersOnly);

        // Construct a term from data previously persisted to a stream via the
        // Write() method.
        Term(std::istream& input);

        // Expands this term's ngram by incorporating another term.
        // Terms must have identical StreamId values. Note that the term hashes
        // are combined using a technique that is not commutative. Therefore
        // a.AddTerm(b) will not be equal to b.AddTerm(b). Therefore care must
        // be taken that phrases are formed the same way during query as they
        // were formed during document ingestion.
        void AddTerm(Term const & term,
                     IConfiguration const & configuration);

        // Equality operator provided for use by unordered_map in Document class.
        bool operator==(const Term& rhs) const;

        // Returns the raw hash value for the term. For most terms, the raw
        // hash is based solely on the term's text.
        // TODO: Explain difference between raw hash and general hash.
        Hash GetRawHash() const;

        // Returns the general hash. This is the raw hash combined with the
        // term's StreamId.
        Hash GetGeneralHash() const;

        // Returns the StreamId of this term.
        StreamId GetStream() const;

        // Returns the Gram size of this term (e.g. unigram = 1, bigram = 2, etc.).
        GramSize GetGramSize() const;

        void Print(std::ostream& output) const;

        // Persists the term to a stream in a form that can be read by
        // Term::Term(std::istream&).
        void Write(std::ostream& output) const;

        // Convert a double precision IDF value to IdfX10.
        static IdfX10 IdfToIdfX10(double idf);

        // Convert an IdfX10 value to frequency.
        static double IdfX10ToFrequency(IdfX10 idf);

        // Convert a frequency at rank 0 to an equivalent frequency at higher
        // rank.
        static double FrequencyAtRank(double frequency, Rank rank);

        // Static method that calculates IDF value from document frequency and
        // corpus document count.
        static IdfX10 ComputeIdfX10(size_t documentFrequency,
                                    double corpusSize,
                                    IdfX10 maxIdf);

        static IdfX10 ComputeIdfX10(double frequency, IdfX10 maxIdf);

        // static method to calculate document frequency from Idf value.
        static unsigned ComputeDocumentFrequency(double corpusSize, double idf);

        // TODO: should some of these static methods move to TermTreatments?
        // Given a term frequency, find the maximum rank such that we don't
        // exceed a density target.
        static Rank ComputeMaxRank(double frequency, double target);

        // Compute the number of rank 0 rows necessary assuming an ideal world
        // (no row correlations) to reach a specified signal to noise ratio
        // (snr).
        static int ComputeRowCount(double frequency, double density, double snr);

        // Computes the general hash, given a raw hash and a classification.
        // Made available as a public static method for use by NGramBuilder.
        static Hash ComputeGeneralHash(Hash hash,
                                       StreamId stream);

        // Computes the raw hash (based on term characters only) for the specified term text.
        static Hash ComputeRawHash(const char* text);

        // Hasher for std::unordered_set.
        // Definition is inlined for use by template.
        struct Hasher
        {
            size_t operator()(Term const & key) const
            {
                return key.GetRawHash();
            }
        };

    private:
        // For now declaring private constructor to prevent use of Term in
        // arrays. If this scenario becomes important, we will revisit the
        // semantics of the default constructor.
        Term();

        // The raw hash for this term. This hash does not incorporate
        // information about the classification.
        Hash m_rawHash;

        // The classification for this term (i.e. metaWord, nonBody, etc.)
        StreamId m_stream;

        // The gramSize for this term (i.e. 2 = bigram)
        GramSize m_gramSize;
    };

    // DESIGN NOTE: intent is that Term is small enough to be used as a value type.
    static_assert(sizeof(Term) <= 16, "sizeof(term) should not exceed 16.");
}
