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

#include "gtest/gtest.h"

#include <sstream>

#include "BitFunnel/Plan/AbstractRow.h"
#include "BitFunnel/Plan/RowMatchNode.h"
#include "BitFunnel/Utilities/Allocator.h"
#include "BitFunnel/Utilities/TextObjectFormatter.h"
#include "TextObjectParser.h"


namespace BitFunnel
{
    namespace AbstractRowUnitTest
    {
        // See https://github.com/BitFunnel/BitFunnel/issues/210
        // void VerifyRoundtripCase(char const * input, char const * expected)
        // {
        //     std::stringstream inputStream(input);
        //     Allocator allocator(1024);
        //     TextObjectParser parser(inputStream, allocator, &RowMatchNode::GetType);
        //     AbstractRow r1(parser, false);

        //     std::stringstream output;
        //     TextObjectFormatter formatter(output);
        //     r1.Format(formatter, nullptr);

        //     EXPECT_EQ(output.str().compare(expected), 0);
        // }


        TEST(AbstractRow,ParseFormat)
        {
            std::stringstream input("AbstractRow(1234, 3");
        }


        TEST(AbstractRow,General)
        {
            AbstractRow r1(1234, 3, true);
            EXPECT_EQ(r1.GetId(), 1234U);
            EXPECT_EQ(r1.GetRank(), 3U);
            EXPECT_EQ(r1.GetRankDelta(), 0U);
            EXPECT_TRUE(r1.IsInverted());
        }


        TEST(AbstractRow,RankUp)
        {
            AbstractRow r0(1234, 6, false);
            AbstractRow r1(r0, 2);

            EXPECT_EQ(r1.GetId(), 1234U);
            EXPECT_EQ(r1.GetRank(), 4U);
            EXPECT_EQ(r1.GetRankDelta(), 2U);
            EXPECT_FALSE(r1.IsInverted());
        }
    }
}
