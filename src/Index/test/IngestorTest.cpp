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


#include <iostream>  // TODO: remove.

#include <cmath>
#include <limits>
#include <memory>
#include <vector>
#include <unordered_map>

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
#include "DocumentFrequencyTable.h"
#include "Primes.h"


namespace BitFunnel
{
    static const Term::StreamId c_streamId = 0;

    class SyntheticIndex
    {
    public:
        SyntheticIndex(unsigned maxDocId, ShardId numShards)
        {
            m_maxDocId = maxDocId;
            m_fileSystem = Factories::CreateFileSystem();
            m_index = Factories::CreatePrimeFactorsIndex(*m_fileSystem,
                                                         m_maxDocId,
                                                         c_streamId,
                                                         numShards);
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
            if (GetIngestor().GetShardCount() == 1)
            {
                // Only doing this for ShardCount == 1 because docIds no longer line up
                // with docIndices if documents can get inserted into any shard.
                for (unsigned i = 0; i < actualMatches.size(); ++i)
                {
                    EXPECT_EQ(actualMatches[i], expectedMatches[i]);
                }
            }
        }

    private:

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
            for (DocId i = 0; i <= m_maxDocId; ++i)
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
        std::vector<DocId> Match(unsigned queryInput)
        {
            std::vector<DocId> results;
            for (ShardId shardId = 0; shardId < GetIngestor().GetShardCount(); ++shardId)
            {
                // Load accumulator with 0xFFFFFFFFFFFFFFFF which matches
                // all documents. Then intersect with rows of the query.
                uint64_t accumulator = std::numeric_limits<uint64_t>::max();
                unsigned query = queryInput;

                for (size_t i = 0; query != 0; query >>= 1, ++i)
                {
                    if (query & 0x1)
                    {
                        char const* text = Primes::c_primesBelow10000Text[i].c_str();

                        Term term(Term::ComputeRawHash(text), c_streamId, 0);
                        RowIdSequence rows(term, m_index->GetTermTable(shardId));
                        for (auto row : rows)
                        {
                            IShard & shard = m_index->GetIngestor().GetShard(shardId);
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
            }
            return results;
        }

        DocId m_maxDocId;
        std::unique_ptr<IFileSystem> m_fileSystem;
        std::unique_ptr<ISimpleIndex> m_index;
    };


    // Generate fake documents where each document contains term i which
    // is a prime number iff the docId is divisible by the prime number.
    // Then verify using a fake matcher that talks to the row table to get
    // the appropriate address for the row.
    TEST(Ingestor, Basic)
    {
        const int c_maxDocId = 63;
        const ShardId c_numShards = 1;
        SyntheticIndex index(c_maxDocId, c_numShards);

        for (unsigned i = 0; i < c_maxDocId + 1; i++)
        {
            index.VerifyQuery(i);
        }
    }


    std::unordered_map<size_t, size_t>
    CreateDocCountHistogram(DocumentFrequencyTable const & table,
                            unsigned docCount)
    {
        std::unordered_map<size_t, size_t> histogram;
        for (size_t i = 0; i < table.size(); ++i)
        {
            auto entry = table[i];
            ++histogram[static_cast<size_t>(std::round(entry.GetFrequency() * docCount))];
        }
        return histogram;
    }

    // Test DocumentFrequencyTable with 3 documents (0, 1, 2). This uses a
    // PrimeFactorIndex, which means that a document contains the term if the
    // term divides the document. This means that the only term we expect is
    // term "2" in docId 2.
    //
    // table.size() == 1u because there's only one term, "2", in any document.
    //
    // docFreqHistogram[1] == 1u because there's exactly 1 term that appears in
    // 1 document.
    TEST(Ingestor, DocFrequency2)
    {
        const int c_maxDocId = 2;
        const ShardId c_numShards = 1;
        SyntheticIndex index(c_maxDocId, c_numShards);
        std::stringstream stream;
        index.GetIngestor().GetShard(0).TemporaryWriteDocumentFrequencyTable(stream, nullptr);

        std::cout << stream.str() << std::endl;

        DocumentFrequencyTable table(stream);

        EXPECT_EQ(table.size(), 1u);
        std::unordered_map<size_t, size_t> docFreqHistogram = CreateDocCountHistogram(table, c_maxDocId);
        EXPECT_EQ(docFreqHistogram[1], 1u);
    }


    // Test DocumentFrequencyTable with 64 documents (0, 1, 2, ..., 63). This
    // uses a PrimeFactorIndex, which means that a document contains the term
    // iff the term divides the document.
    //
    // table.size() == 18u because there are 18 primes <= 63.
    //
    // docFreqHistogram[31] == 1u because there's exactly 1 term that appears in
    // 31 documents: the term "2" appears in documents 2, 4, 6, 8, ..., 62.
    //
    // docFreqHistogram[1] == 7u because there are 7 terms that appear in
    // exactly 1 document: 37, 41, 43, 47, 53, 59, 61, i.e., all primes between
    // 32 and 63 (inclusive).
    TEST(Ingestor, DocFrequency63)
    {
        const int c_maxDocId = 63;
        const ShardId c_numShards = 1;
        SyntheticIndex index(c_maxDocId, c_numShards);
        std::stringstream stream;
        index.GetIngestor().GetShard(0).TemporaryWriteDocumentFrequencyTable(stream, nullptr);

        std::cout << stream.str() << std::endl;

        DocumentFrequencyTable table(stream);

        EXPECT_EQ(table.size(), 18u);
        std::unordered_map<size_t, size_t> docFreqHistogram = CreateDocCountHistogram(table, c_maxDocId);

        EXPECT_EQ(docFreqHistogram[21], 1u);
        EXPECT_EQ(docFreqHistogram[31], 1u);
    }


    TEST(Ingestor, BasicMultiShard)
    {
        const int c_maxDocId = 63;
        // Note: we'll need to allocate more memory if we want to add more
        // shards.
        for (ShardId numShards = 1; numShards < 4; ++numShards)
        {
            SyntheticIndex index(c_maxDocId, numShards);

            // We don't look at docId 0 because it matches everything, which
            // will cause it to match all documents in all shards. It's possible
            // that we should fix that by changing how PrimeFactorsDocument
            // works.
            for (unsigned i = 1; i < c_maxDocId + 1; i++)
            {
                std::cout << i << std::endl;
                index.VerifyQuery(i);
            }
        }
    }

}
