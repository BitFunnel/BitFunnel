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
#include "BitFunnel/Index/IConfiguration.h"
#include "BitFunnel/Utilities/IObjectParser.h"
#include "LoggerInterfaces/Logging.h"
#include "MurmurHash2.h"
#include "TermToText.h"


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
               IConfiguration const & configuration)
        : m_rawHash(ComputeRawHash(text)),
          m_stream(stream),
          m_gramSize(1)
    {
        // If we're maintaining a term-to-text mapping.
        if (configuration.KeepTermText())
        {
            auto & termToText = configuration.GetTermToText();

            // If this term isn't already in the table.
            auto const termText = termToText.Lookup(m_rawHash);
            if (termText.size() == 0)
            {
                // Add the term to the table.
                termToText.AddTerm(m_rawHash, std::string(text));
            }
        }
    }


    Term::Term(Hash rawHash,
               StreamId stream,
               GramSize gramSize)
        : m_rawHash(rawHash),
          m_stream(stream),
          m_gramSize(gramSize)
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


    static char const * c_termPrimitiveName = "Term";
    Term::Term(IObjectParser& parser, bool parseParametersOnly)
    {
        if (!parseParametersOnly)
        {
            parser.OpenPrimitive(c_termPrimitiveName);
        }

        LogAssertB(parser.OpenPrimitiveItem(), "");
        m_rawHash = parser.ParseUInt64();

        LogAssertB(parser.OpenPrimitiveItem(), "");
        // TODO: check for cast overflow?
        m_stream = static_cast<StreamId>(parser.ParseUInt64());

        LogAssertB(parser.OpenPrimitiveItem(), "");
        unsigned gramSize = parser.ParseUInt();
        LogAssertB(gramSize <= c_maxGramSize, "");
        m_gramSize = static_cast<uint8_t>(gramSize);

        // TODO: Decide whether we really want to keep maxWordIdf.
        // For now, disable serialization to work around unit test breaks.
        //LogAssertB(parser.OpenPrimitiveItem());
        //unsigned maxWordIdf = parser.ParseUInt();
        //LogAssertB(maxWordIdf <= c_maxIdfX10Value);
        //m_maxWordIdf = static_cast<IdfX10>(maxWordIdf);

        // Commenting out the code related to tier since we don't have a concept
        // of tier anymore
        // LogAssertB(parser.OpenPrimitiveItem());
        // std::string tierName;
        // parser.ParseToken(tierName);
        // m_tierHint = StringToTier(tierName);

        if (!parseParametersOnly)
        {
            parser.ClosePrimitive();
        }
    }

    void Term::AddTerm(Term const & term,
                       IConfiguration const & configuration)
    {
        if (m_stream != term.m_stream)
        {
            throw FatalError("Attempting to combine terms with different streams.");
        }

        Hash leftHash = m_rawHash;

        m_rawHash = rotl64By1(m_rawHash) ^ term.m_rawHash;
        m_gramSize += term.m_gramSize;


        // If we're maintaining a term-to-text mapping.
        if (configuration.KeepTermText())
        {
            auto & termToText = configuration.GetTermToText();

            // If this phrase isn't already in the table.
            if (termToText.Lookup(m_rawHash).size() == 0)
            {
                // Concatenate old text with new text.
                auto const left = termToText.Lookup(leftHash);
                auto const right = termToText.Lookup(term.m_rawHash);
                std::string phrase;
                phrase.reserve(left.size() + 1 + right.size());
                phrase.append(left);
                phrase.push_back(' ');
                phrase.append(right);

                // Add the phrase to the IDF table.
                termToText.AddTerm(m_rawHash, phrase);
            }
        }
    }


    bool Term::operator==(const Term& other) const
    {
        return m_rawHash == other.m_rawHash
            && m_gramSize == other.m_gramSize;
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


    double Term::IdfX10ToFrequency(IdfX10 idf)
    {
        return pow(10, -idf / 10.0);
    }


    double Term::FrequencyAtRank(double frequency, Rank rank)
    {
        // We special case rank0 in order to avoid floating point rounding at
        // rank 0.
        if (rank == 0)
        {
            return frequency;
        }
        else
        {
            const size_t rowCount = (1ull << rank);
            return 1.0 - pow(1.0 - frequency, rowCount);
        }
    }


    unsigned Term::ComputeDocumentFrequency(double corpusSize, double idf)
    {
        return static_cast<unsigned>(corpusSize / pow(10, idf));
    }


    Rank Term::ComputeMaxRank(double frequency, double target)
    {
        if (frequency <= 0 || target >= 1.0)
        {
            // TODO: throw here?
            return 0;
        }

        Rank result = 0;
        while (FrequencyAtRank(frequency, result) < target)
        {
            ++result;
        }
        return result;
    }


    int Term::ComputeRowCount(double frequency, double density, double snr)
    {
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#endif
                return lround(ceil(log(frequency / snr) / log(density - frequency)));
#ifdef __clang__
#pragma clang diagnostic pop
#endif
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
        // TODO: Need some means to ensure that a term never gets the same hash
        // as a system row or a fact.
        const int c_murmurHashSeedForText = 123456789;

        Hash hash = MurmurHash64A(text, strlen(text), c_murmurHashSeedForText);

        return hash;
    }
}
