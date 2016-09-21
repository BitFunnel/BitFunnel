#include "gtest/gtest.h"

#include <sstream>

#include "Allocator.h"
#include "BitFunnel/Plan/AbstractRow.h"
#include "BitFunnel/Plan/RowMatchNode.h"
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
