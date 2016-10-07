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

#include <limits>
#include <memory>
#include <vector>

#include "gtest/gtest.h"

#include "BitFunnel/BitFunnelTypes.h"
#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/IShard.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Index/RowIdSequence.h"
#include "BitFunnel/Mocks/Factories.h"
#include "BitFunnel/Term.h"
#include "Primes.h"


namespace BitFunnel
{
    static const Term::StreamId c_streamId = 0;


    class SyntheticIndex
    {
    public:
        SyntheticIndex(unsigned documentCount)
        {
            m_fileSystem = Factories::CreateFileSystem();
            m_index = Factories::CreatePrimeFactorsIndex(*m_fileSystem,
                                                         documentCount,
                                                         c_streamId);
        }


        IIngestor & GetIngestor() const
        {
            return m_index->GetIngestor();
        }


        void VerifyQuery(unsigned query)
        {
            auto actualMatches = Match(query);
            auto expectedMatches = Expected(query);

            ASSERT_EQ(actualMatches.size(), expectedMatches.size());
            for (unsigned i = 0; i < actualMatches.size(); ++i)
            {
                    EXPECT_EQ(actualMatches[i], expectedMatches[i]);
            }
        }

    private:
        static const DocId c_documentCount = 64;

        bool ExpectedMatch(DocId id, unsigned query)
        {
            if (query == 0)
            {
                // Query 0 has no terms so it matches all documents.
                return true;
            }
            else if (id == 0)
            {
                // Document 0 has no terms so it never matches
                // a non-zero query.
                return false;
            }
            else
            {
                // Document and query each have at least one term.
                // Check for matches.
                for (size_t i = 0; query != 0; query >>= 1, ++i)
                {
                    if ((query & 1) != 0)
                    {
                        if (id % Primes::c_primesBelow10000[i] != 0)
                        {
                            return false;
                        }
                    }
                }
                return true;
            }
        }


        std::vector<DocId> Expected(unsigned query)
        {
            std::vector<DocId> results;
            for (DocId i = 0; i < c_documentCount; ++i)
            {
                if (ExpectedMatch(i, query))
                {
                    results.push_back(i);
                }
            }
            return results;
        }


        // Start with an accumulator that matches all documents. Then
        // intersect rows as appropriate. This implies that the "0" query
        // matches all rows. Note that this only handles up to 64 bits, so
        // queries larger than 64 are bogus.
        std::vector<DocId> Match(unsigned query)
        {
            // Load accumulator with 0xFFFFFFFFFFFFFFFF which matches
            // all documents. Then intersect with rows of the query.
            uint64_t accumulator = std::numeric_limits<uint64_t>::max();
            std::vector<DocId> results;

            for (size_t i = 0; query != 0; query >>= 1, ++i)
            {
                if (query & 0x1)
                {
                    char const* text = Primes::c_primesBelow10000Text[i].c_str();

                    Term term(Term::ComputeRawHash(text), c_streamId, 0);
                    RowIdSequence rows(term, m_index->GetTermTable());
                    for (auto row : rows)
                    {
                        IShard & shard = m_index->GetIngestor().GetShard(0);
                        auto rowOffset = shard.GetRowOffset(row);
                        auto sliceBuffers = shard.GetSliceBuffers();
                        auto base = static_cast<char*>(sliceBuffers[0]);
                        auto ptr = base + rowOffset;
                        accumulator &= *reinterpret_cast<uint64_t*>(ptr);
                    }
                }
            }

            for (unsigned i = 0; accumulator != 0; ++i, accumulator >>= 1)
            {
                if (accumulator & 1)
                {
                    results.push_back(i);
                }
            }
            return results;
        }

        std::unique_ptr<IFileSystem> m_fileSystem;
        std::unique_ptr<ISimpleIndex> m_index;
    };


    // Generate fake documents where each document contains term i which
    // is a prime number iff the docId is divisible by the prime number.
    // Then verify using a fake matcher that talks to the row table to get
    // the appropriate address for the row.
    TEST(Ingestor, Basic)
    {
        const int c_documentCount = 64;
        SyntheticIndex index(c_documentCount);

        for (unsigned i = 0; i < c_documentCount + 1; i++)
        {
            index.VerifyQuery(i);
        }
    }


    /*
      std::unordered_map<size_t, size_t>
      CreateDocCountHistogram(DocumentFrequencyTable const & table,
      unsigned docCount)
      {
      std::unordered_map<size_t, size_t> histogram;
      for (size_t i = 0; i < table.size(); ++i)
      {
      auto entry = table[i];
      ++histogram[static_cast<size_t>(round(entry.GetFrequency() * docCount))];
      }
      return histogram;
      }


      // Ingest fake documents as in "Basic" test, then print statistics out
      // to a stream. Verify the statistics by reading them out as a
      // stream. Verify the statistics by reading them into the
      // DocumentFrequencyTable constructor and checking the
      // DocumentFrequencyTable.
      TEST(Ingestor, DocFrequency64)
      {
      const int c_documentCount = 64;
      SyntheticIndex index(c_documentCount);
      std::stringstream stream;
      index.GetIngestor().GetShard(0).TemporaryWriteDocumentFrequencyTable(stream, nullptr);

      std::cout << stream.str() << std::endl;

      DocumentFrequencyTable table(stream);

      EXPECT_EQ(table.size(), 6u);
      std::unordered_map<size_t, size_t> docFreqHistogram = CreateDocCountHistogram(table, c_documentCount);
      EXPECT_EQ(docFreqHistogram[32], 6u);
      }


      TEST(Ingestor, DocFrequency63)
      {
      const int c_documentCount = 63;
      SyntheticIndex index(c_documentCount);
      std::stringstream stream;
      index.GetIngestor().GetShard(0).TemporaryWriteDocumentFrequencyTable(stream, nullptr);

      DocumentFrequencyTable table(stream);

      EXPECT_EQ(table.size(), 6u);
      std::unordered_map<size_t, size_t> docFreqHistogram = CreateDocCountHistogram(table, c_documentCount);
      EXPECT_EQ(docFreqHistogram[31], 6u);
      }


      TEST(Ingestor, DocFrequency62)
      {
      const int c_documentCount = 62;
      SyntheticIndex index(c_documentCount);
      std::stringstream stream;
      index.GetIngestor().GetShard(0).TemporaryWriteDocumentFrequencyTable(stream, nullptr);

      DocumentFrequencyTable table(stream);

      EXPECT_EQ(table.size(), 6u);
      std::unordered_map<size_t, size_t> docFreqHistogram = CreateDocCountHistogram(table, c_documentCount);
      EXPECT_EQ(docFreqHistogram[30], 5u);
      EXPECT_EQ(docFreqHistogram[31], 1u);
      }
    */
}
