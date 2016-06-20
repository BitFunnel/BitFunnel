#pragma once

#include <iosfwd>       // TODO: Remove this. For temporary Print().
#include <stdint.h>     // For uint8_t


namespace BitFunnel
{
    class IDocumentFrequencyTable;
    class NGramBuilder;

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
        typedef uint8_t GramSize;
        typedef size_t Hash;
        typedef uint8_t IdfX10;
        typedef uint8_t StreamId;


        static const GramSize c_log2MaxGramSize = 3;
        static const GramSize c_maxGramSize = 1 << c_log2MaxGramSize;
        static const IdfX10 c_maxIdfX10Value = 60;

        // TODO: Should terms store IDF values? Why?
        // TODO: Should terms store ngram sizes? Why?

        // Creates a Term for a unigram based on its raw hash
        // and classification. Its document frequency provides the term's
        // IdfSum value. The IMetaWordTierHintMap is used to set the term's
        // TierHint value.
        Term(char const * text,
             StreamId stream,
             IDocumentFrequencyTable const & docFreqTable);

        void AddTerm(Term const & term);

        // Equality operator provided for use by unordered_map in Document class.
        bool operator==(const Term& rhs) const;

        // Returns the raw hash value for the term. For most terms, the raw
        // hash is based solely on the term's text. Terms with classification
        // Click and ClickExperimental also incorporate the stream suffix in
        // the raw hash value.
        Hash GetRawHash() const;

        // Returns the general hash. This is the raw hash combined with the
        // term's StreamId.
        Hash GetGeneralHash() const;

        // Returns the StreamId of this term.
        StreamId GetStream() const;

        // Returns the Gram size of this term (e.g. bigram = 2).
        GramSize GetGramSize() const;

        // Returns the term's IDF Sum value. IDF stands for inverse document
        // frequency. It is defined as the log base-10 of ratio of the corpus
        // size to the number of times the term appears in a corpus. As an
        // example, a term that appears once in every 1000 documents will
        // have an IDF of 3. GetIdfSum() returns the IDF Sum as an IdfX10 value.
        // IdfX10 is a one-byte size that is equal to the actual IDF value
        // multiplied by 10 and rounded to the nearest integer. For more
        // information see
        //      http://en.wikipedia.org/wiki/Inverse_document_frequency
        // For ngram, Idf sum is the sum of all its unigrams' IDF.
        IdfX10 GetIdfSum() const;

        // Returns the max IDF value of all words in this term.
        IdfX10 GetIdfMax() const;

        void Print(std::ostream& output) const;

        // Convert a double precision IDF value to IdfX10
        static IdfX10 IdfToIdfX10(double idf);

        // Static method that calculates IDF value from document frequency and
        // corpus document count.
        static IdfX10 ComputeIdfX10(size_t documentFrequency,
                                    double corpusSize,
                                    IdfX10 maxIdf);

        // static method to calculate document frequency from Idf value.
        static unsigned ComputeDocumentFrequency(double corpusSize, double idf);

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

        // An approximation of the IDF value for the term.
        // If gram size is 1, it is the real IDF value of the term.
        IdfX10 m_idfSum;

        // Max IDF value of all words in this term. 
        // The real IDF value of this term should be bigger than this value.
        IdfX10 m_idfMax;
    };

    // DESIGN NOTE: intent is that Term is small enough to be used as a value type.
    static_assert(sizeof(Term) <= 16, "sizeof(term) should not exceed 16.");
}
