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
#include "BitFunnel/Index/IIndexedIdfTable.h"
#include "Document.h"


namespace BitFunnel
{
    TEST(Document, ContainsTerm)
    {
        const Term::StreamId streamId = 0;
        const DocId docId = 0;

        auto idfTable = Factories::CreateIndexedIdfTable();
        auto config = 
            Factories::CreateConfiguration(1, false, *idfTable);
        Document d(*config, docId);

        std::array<char const *, 5> text = {
            "one",
            "two",
            "three",
            "four",
            "five"
        };

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

        Term unexpected("unexpected", streamId, *config);
        EXPECT_FALSE(d.Contains(unexpected));
    }
}
