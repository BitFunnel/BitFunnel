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

#include <sstream>

#include "gtest/gtest.h"

#include "DocumentFrequencyTable.h"
#include "TermToText.h"


namespace BitFunnel
{
    namespace DocumentFrequencyTableTest
    {
        static DocumentFrequencyTable::Entry MakeEntry(Term::Hash hash)
        {
            Term::StreamId streamId = hash % 4;
            Term::IdfX10 idf = hash % 5;
            Term::GramSize gramSize = hash % 6;
            double frequency = Term::IdfX10ToFrequency(idf);
            std::string text = std::to_string(hash);

            Term term(hash, streamId, gramSize);
            DocumentFrequencyTable::Entry entry(term, frequency);

            return entry;
        }


        // Add then verify mappings added to a DocumentFrequencyTable. 
        TEST(DocumentFrequencyTable, AddTerm)
        {
            DocumentFrequencyTable terms;

            Term::Hash maxHash = 100;
            for (Term::Hash hash = 0; hash < maxHash; ++hash)
            {
                terms.AddEntry(MakeEntry(hash));
            }

            for (Term::Hash hash = 0; hash < maxHash; ++hash)
            {
                DocumentFrequencyTable::Entry expected = MakeEntry(hash);
                auto observed = terms[hash];

                EXPECT_EQ(observed, expected);
            }
        }


        // Add then verify mappings with TermToText. Then verify stream output.
        TEST(DocumentFrequencyTable, AddTermWithTermToText)
        {
            TermToText termToText;
            termToText.AddTerm(0ull, "term0");
            termToText.AddTerm(1ull, "term1");

            DocumentFrequencyTable terms;

            Term::Hash maxHash = 3;
            for (Term::Hash hash = 0; hash < maxHash; ++hash)
            {
                terms.AddEntry(MakeEntry(hash));
            }

            std::stringstream stream;
            terms.Write(stream, &termToText);

            char const * expected =
                "hash,gramSize,streamId,frequency,text\n"
                "0,0,0,1,term0\n"
                "1,1,1,0.794328,term1\n"
                "2,2,2,0.630957,\n";

            // NOTE: need to save the std::string, not its c_str because the
            // c_str value will go away when the std::string is destructed.
            auto observed = stream.str();

            EXPECT_STREQ(expected, observed.c_str());
        }


        // Write DocumentFrequencyTable to stream, then construct new
        // DocumentFrequencyTable from stream and verify contents.
        TEST(DocumentFrequencyTable, RoundTrip)
        {
            DocumentFrequencyTable terms;

            Term::Hash maxHash = 100;
            for (Term::Hash hash = 0; hash < maxHash; ++hash)
            {
                terms.AddEntry(MakeEntry(hash));
            }

            std::stringstream stream;
            terms.Write(stream, nullptr);

            DocumentFrequencyTable terms2(stream);

            // Table will now be sorted. Need to track whether each
            // original entry was found, and whether all its fields are
            // correct.
            std::vector<bool> found(maxHash, false);
            for (auto observed : terms)
            {
                // Verify that we haven't seen this hash already.
                auto hash = observed.GetTerm().GetRawHash();
                EXPECT_FALSE(found[hash]);

                // Mark that we've seen this hash.
                found[hash] = true;

                // Verify that the Entry has the expected values.
                auto expected = MakeEntry(hash);
                EXPECT_EQ(observed, expected);
            }
        }
    }
}
