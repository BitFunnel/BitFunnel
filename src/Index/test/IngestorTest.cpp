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

// #include "gtest/gtest.h"


// namespace BitFunnel
// {
//     TEST(Ingestor, Placeholder)
//     {
//     }
// }


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

#include <iostream>
#include <cmath>
#include <limits>
#include <memory>
#include <sstream>
#include <string>

#include "gtest/gtest.h"

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/IRecycler.h"
#include "Configuration.h"
#include "Document.h"
#include "DocumentDataSchema.h"
#include "DocumentFrequencyTable.h"
#include "IndexedIdfTable.h"
// #include "IndexUtils.h"
#include "Ingestor.h"
// #include "MockFileManager.h"
// #include "MockTermTable.h"
#include "Recycler.h"
// #include "BitFunnel/TermInfo.h"
#include "BitFunnel/Index/RowIdSequence.h"
#include "BitFunnel/Term.h"
#include "TrackingSliceBufferAllocator.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Index/ITermTable.h"
#include "Primes.h"

namespace BitFunnel
{
    class IConfiguration;

    // Documents are generated using the following mapping: A document contains
    // term i iff bit i of the docId is set.

    namespace IngestorTest
    {
        // const size_t c_maxGramSize = 1;
        // const Term::StreamId c_streamId = 0;


        std::vector<std::string> GenerateDocumentText(unsigned docId)
        {
            std::vector<std::string> terms;
            for (int i = 0; i < 32 && docId != 0; ++i, docId >>= 1)
            {
                if (docId & 1)
                {
                    terms.push_back(std::to_string(i));
                }
            }
            return terms;
        }


        // Contains an Index, as well as other things necessary for an index,
        // such as a Recycler.
        class IndexWrapper
        {
        public:
            IndexWrapper()
            {
                m_fileSystem = Factories::CreateFileSystem();
                m_index = Factories::CreateSimpleIndex(*m_fileSystem);

                m_index->ConfigureAsMock(1, false);

                m_index->StartIndex();

            }

            ~IndexWrapper()
            {
            }

            IIngestor & GetIngestor() const
            {
                return m_index->GetIngestor();
            }

            ITermTable & GetTermTable() const
            {
                return *m_termTable;
            }

        private:
            // std::unique_ptr<TrackingSliceBufferAllocator> m_allocator;
            // std::unique_ptr<IIngestor> m_ingestor;
            std::unique_ptr<ISimpleIndex> m_index;
            std::unique_ptr<ITermTable> m_termTable;
            std::unique_ptr<IFileSystem> m_fileSystem;
            // std::unique_ptr<IRecycler> m_recycler;
            // std::future<void> m_recyclerHandle;
            // std::unique_ptr<IShardDefinition> m_shardDefinition;
        };


        class SyntheticIndex {
        public:
            SyntheticIndex(unsigned /*documentCount*/)
                : m_index()
            {}

            IIngestor & GetIngestor() const
            {
                return m_index->GetIngestor();
            }

            void VerifyQuery(unsigned query)
            {
                auto actualMatches = Match(query, *m_index);
                auto expectedMatches = Expected(query);
                ASSERT_EQ(actualMatches.size(), expectedMatches.size());
                for (unsigned i = 0; i < actualMatches.size(); ++i)
                    {
                        EXPECT_EQ(actualMatches[i], expectedMatches[i]);
                    }
            }

        private:
            const DocId documentCount = 64;

            bool ExpectedMatch(DocId id, unsigned query) {
                // Primes::c_primesBelow10000
                for (size_t i = 0; query != 0; query >>= 1, ++i) {
                    if (query & 0x1) {
                        if (id % Primes::c_primesBelow10000[i] != 0) {
                            return false;
                        }
                    }
                }
                return true;
            }

            std::vector<unsigned> Expected(unsigned query) {
                std::vector<unsigned> results;
                for (DocId i = 0; i < documentCount; ++i) {
                    if (ExpectedMatch(i, query)) {
                        std::cout << "ID " << i << " query " << query << '\n';
                        results.push_back(i);
                    }
                }
                return results;
            }
            std::vector<unsigned> Match(unsigned /*query*/,
                                        IndexWrapper const & /*index*/) { return std::vector<unsigned>(); }

            std::unique_ptr<IndexWrapper> m_index;
        };


        // Generate fake documents where each document contains term i iff the
        // docId has bit i set, and then verify using a fake matcher that talks
        // to the row table to get the appropriate address for the row.
        TEST(Ingestor, Basic)
        {
            const int c_documentCount = 64;
            SyntheticIndex index(c_documentCount);

            for (int i = 0; i < c_documentCount + 1; i++)
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
}
