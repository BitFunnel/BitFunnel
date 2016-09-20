#include "stdafx.h"

#include <sstream>

#include "BitFunnel/AbstractRow.h"
#include "PrivateHeapAllocator.h"
#include "BitFunnel/RowMatchNodes.h"
#include "TextObjectFormatter.h"
#include "TextObjectParser.h"
#include "SuiteCpp/UnitTest.h"


namespace BitFunnel
{
    namespace AbstractRowUnitTest
    {
        void VerifyRoundtripCase(char const * input, char const * expected)
        {
            std::stringstream inputStream(input);
            PrivateHeapAllocator allocator;
            TextObjectParser parser(inputStream, allocator, &RowMatchNode::GetType);
            AbstractRow r1(parser, false);

            std::stringstream output;
            TextObjectFormatter formatter(output);
            r1.Format(formatter, nullptr);

            TestAssert(output.str().compare(expected) == 0);
        }


        TestCase(ParseFormat)
        {
            std::stringstream input("AbstractRow(1234, 3");
        }


        TestCase(General)
        {
            AbstractRow r1(1234, 3, true);
            TestEqual(r1.GetId(), 1234U);
            TestEqual(r1.GetRank(), 3U);
            TestEqual(r1.GetRankDelta(), 0U);
            TestAssert(r1.IsInverted());
        }


        TestCase(RankUp)
        {
            AbstractRow r0(1234, 6, false);
            AbstractRow r1(r0, 2);

            TestEqual(r1.GetId(), 1234U);
            TestEqual(r1.GetRank(), 4U);
            TestEqual(r1.GetRankDelta(), 2U);
            TestAssert(!r1.IsInverted());
        }
    }
}
