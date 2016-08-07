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

#include <algorithm>        // For std::min.
#include <cstring>          // For strlen.
#include <istream>
#include <math.h>           // For log10.

#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Term.h"
#include "LoggerInterfaces/Logging.h"
#include "MurmurHash2.h"


namespace BitFunnel
{
    static uint64_t rotl64By1(uint64_t x)
    {
        // TODO: Investigate `_rotl64` intrinsics; this exists on Windows, but
        // does not seem to on Clang.
        return (x << 1) | (x >> 63);
    }


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


    Term::Term(Hash rawHash,
               StreamId stream,
               IdfX10 idf)
        : m_rawHash(rawHash),
          m_stream(stream),
          m_gramSize(1),
          m_idfSum(idf),
          m_idfMax(idf)
    {
    }


    Term::Term(std::istream& input)
    {
        unsigned temp;
        char comma;
        input >> std::hex >> m_rawHash;
        input >> comma;
        LogAssertB(comma == ',', "bad input format.");
        input >> std::dec >>temp;
        m_gramSize = static_cast<GramSize>(temp);
        input >> comma;
        LogAssertB(comma == ',', "bad input format.");
        input >> std::dec >>temp;
        m_stream = static_cast<StreamId>(temp);
    }


    void Term::AddTerm(Term const & term)
    {
        if (m_stream != term.m_stream)
        {
            throw FatalError("Attempting to combine terms with different streams.");
        }

        m_rawHash = rotl64By1(m_rawHash) ^ term.m_rawHash;
        m_gramSize += term.m_gramSize;
        m_idfSum += term.m_idfSum;
        m_idfMax = std::max(m_idfMax, term.m_idfMax);
    }


    bool Term::operator==(const Term& other) const
    {
        return m_rawHash == other.m_rawHash
            && m_gramSize == other.m_gramSize
            && m_idfSum == other.m_idfSum
            && m_idfMax == other.m_idfMax;
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
            << std::dec << static_cast<unsigned>(m_gramSize)
            << ", "
            << static_cast<unsigned>(m_stream)
            << ")";
    }


    void Term::Write(std::ostream& out) const
    {
        out << std::hex << m_rawHash
            << ","
            << std::dec << static_cast<unsigned>(m_gramSize)
            << ","
            << static_cast<unsigned>(m_stream);
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


    Term::IdfX10 Term::ComputeIdfX10(double frequency, IdfX10 maxIdf)
    {
        if (frequency == 0)
        {
            return static_cast<IdfX10>(maxIdf);
        }
        else
        {
            IdfX10 idf = IdfToIdfX10(log10(1.0 / frequency));

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
