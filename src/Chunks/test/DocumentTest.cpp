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

#include <array>

#include "gtest/gtest.h"

#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IConfiguration.h"
#include "Document.h"


namespace BitFunnel
{
    TEST(Document, ContainsTerm)
    {
        const Term::StreamId streamId = 0;
        const DocId docId = 0;
        const size_t gramSize = 1;

        auto facts = Factories::CreateFactSet();
        auto config =
            Factories::CreateConfiguration(gramSize, false, *facts);
        Document d(*config, docId);

        std::array<char const *, 5> text {{
            "one",
            "two",
            "three",
            "four",
            "five"
         }};

        d.OpenStream(streamId);
        for (auto word : text)
        {
            d.AddTerm(word);
        }
        d.CloseStream();
        d.CloseDocument(0);

        for (auto word : text)
        {
            Term term(word, streamId, *config);
            EXPECT_TRUE(d.Contains(term));
        }

        // TODO: check other 2-grams.
        Term twoGram(text[0], streamId, *config);
        Term partTwo(text[1], streamId, *config);
        twoGram.AddTerm(partTwo, *config);
        EXPECT_FALSE(d.Contains(twoGram));

        Term unexpected("unexpected", streamId, *config);
        EXPECT_FALSE(d.Contains(unexpected));
    }


    TEST(Document, containsNGram)
    {
        const Term::StreamId streamId = 0;
        const DocId docId = 0;
        const size_t gramSize = 5;

        auto facts = Factories::CreateFactSet();
        auto config =
            Factories::CreateConfiguration(gramSize, false, *facts);
        Document d(*config, docId);

        std::array<char const *, 5> text {{
            "one",
            "two",
            "three",
            "four",
            "five"
         }};

        d.OpenStream(streamId);
        for (auto word : text)
        {
            d.AddTerm(word);
        }
        d.CloseStream();
        d.CloseDocument(0);

        // Check for each N-gram.
        for (size_t i = 0; i < text.size(); ++i)
        {
            Term term(text[i], streamId, *config);
            for (size_t j = i+1; j < text.size(); ++j)
            {
                Term subTerm(text[j], streamId, *config);
                term.AddTerm(subTerm, *config);
                EXPECT_TRUE(d.Contains(subTerm));
                EXPECT_TRUE(d.Contains(term));
            }
        }

        // Check that reverse N-grams aren't included.
        for (int i = static_cast<int>(text.size())-1; i >= 0; --i)
        {
            Term term(text[static_cast<size_t>(i)], streamId, *config);
            for (int j = i-1; j >= 0; --j)
            {
                Term subTerm(text[static_cast<size_t>(j)], streamId, *config);
                term.AddTerm(subTerm, *config);
                EXPECT_FALSE(d.Contains(term));
            }
        }

        Term unexpected("unexpected", streamId, *config);
        EXPECT_FALSE(d.Contains(unexpected));
    }
}
