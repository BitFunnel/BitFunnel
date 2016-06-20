#include <algorithm>        // For std::min.
#include <math.h>           // For log10.
#include <ostream>          // Print() uses std::ostream.

#include "BitFunnel/Exceptions.h"
#include "Term.h"
#include "LoggerInterfaces/Logging.h"
#include "MurmurHash2.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // Term
    //
    //*************************************************************************
    Term::Term(char const * text,
               StreamId stream,
               IDocumentFrequencyTable const & /*docFreqTable*/)
        : m_rawHash(ComputeRawHash(text)),
          m_stream(stream),
          m_gramSize(1),
          m_idfSum(0),      // TODO: use correct value
          m_idfMax(0)       // TODO: use correct value
    {
    }


    void Term::AddTerm(Term const & term)
    {
        if (m_stream != term.m_stream)
        {
            throw FatalError("Attempting to combine terms with different streams.");
        }
        m_rawHash = _rotl64(m_rawHash, 1) ^ term.m_rawHash;
        m_gramSize += term.m_gramSize;
        m_idfSum += term.m_idfSum;
        m_idfMax = std::max(m_idfMax, term.m_idfMax);
    }


    Term::Hash Term::GetRawHash() const
    {
        return m_rawHash;
    }


    Term::Hash Term::GetGeneralHash() const
    {
        return m_rawHash + static_cast<Hash>(m_stream);
    }


    Term::StreamId Term::GetStream() const
    {
        return m_stream;
    }


    Term::GramSize Term::GetGramSize() const
    {
        return m_gramSize;
    }


    Term::IdfX10 Term::GetIdfSum() const
    {
        return m_idfSum;
    }


    Term::IdfX10 Term::GetIdfMax() const
    {
        return m_idfMax;
    }


    void Term::Print(std::ostream& out) const
    {
        out << "Term("
            << std::hex << m_rawHash
            << ", "
            << std::dec << (size_t)m_gramSize
            << ")";
    }


    Term::IdfX10 Term::ComputeIdfX10(size_t documentFrequency,
                                     double corpusSize,
                                     IdfX10 maxIdf)
    {
        if (documentFrequency == 0)
        {
            return static_cast<IdfX10>(maxIdf);
        }
        else
        {
            IdfX10 idf = IdfToIdfX10(log10(corpusSize / documentFrequency));

            return (std::min)(maxIdf, (std::min)(static_cast<IdfX10>(c_maxIdfX10Value), idf));
        }
    }


    unsigned Term::ComputeDocumentFrequency(double corpusSize, double idf)
    {
        return static_cast<unsigned>(corpusSize / pow(10, idf));
    }


    Term::IdfX10 Term::IdfToIdfX10(double idf)
    {
        return static_cast<IdfX10>(idf * 10.0 + 0.5);
    }


    Term::Hash Term::ComputeGeneralHash(Hash hash,
                                        StreamId stream)
    {
        return hash + static_cast<Hash>(stream);
    }


    Term::Hash Term::ComputeRawHash(char const * text)
    {
        const int c_murmurHashSeedForText = 123456789;

        Hash hash = MurmurHash64A(text, strlen(text), c_murmurHashSeedForText);

        return hash;
    }
}
