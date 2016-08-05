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

#pragma warning(push)
#pragma warning(disable:4996)

#include <algorithm>        // std::equal
#include <deque>
#include <iostream>         // TODO: remove this temporary include.

#include "BitFunnel/Exceptions.h"
#include "ITermTreatment.h"
#include "gtest/gtest.h"


namespace BitFunnel
{
    namespace RowConfigurationTest
    {
        // Create RowConfiguration::Entry and get fields.
        TEST(Entry, Construct)
        {
            RowConfiguration::Entry e1(1u, 2u, false);
            EXPECT_EQ(e1.GetRank(), 1u);
            EXPECT_EQ(e1.GetRowCount(), 2u);
            EXPECT_FALSE(e1.IsPrivate());

            RowConfiguration::Entry e(7u, 6u, true);
            EXPECT_EQ(e.GetRank(), 7u);
            EXPECT_EQ(e.GetRowCount(), 6u);
            EXPECT_TRUE(e.IsPrivate());
        }


        // Throw on invalid RowConfiguration::Entry.
        TEST(Entry, Throw)
        {
            // Rank is too large.
            ASSERT_THROW(RowConfiguration::Entry(8u, 1u, true),
                         RecoverableError);

            // RowCount is too large.
            ASSERT_THROW(RowConfiguration::Entry(0u, 16u, true),
                         RecoverableError);

            // RowCount is zero.
            ASSERT_THROW(RowConfiguration::Entry(1u, 0u, true),
                         RecoverableError);
        }


        // Throw on iterate empty.
        TEST(RowConfiguration, ThrowOnIterateEmpty)
        {
            RowConfiguration c;
            auto it = c.begin();

            // Shouldn't be able to dereference iterator to empty RowConfiguration.
            ASSERT_THROW(*it, RecoverableError);

            // Shouldn't be able to advance iterator to empty RowConfiguration.
            ASSERT_THROW(++it, RecoverableError);
        }

        // Iterate correct number.
        // push_front
        // Throw on push_front full.
        // Throw on duplicate rank
        TEST(RowConfiguration, Iterate)
        {
            std::deque<RowConfiguration::Entry> expected;
            RowConfiguration observed;

            for (unsigned i = 0; i < 8; ++i)
            {
                RowConfiguration::Entry e(i, i + 1, (i & 1) == 1);
                expected.push_front(e);
                observed.push_front(e);

                EXPECT_TRUE(std::equal(observed.begin(), observed.end(), expected.begin()));
                EXPECT_TRUE(std::equal(expected.begin(), expected.end(), observed.begin()));
                //size_t j = 0;
                //for (auto e : observed)
                //{
                //    EXPECT_EQ(expected[j], e);
                //    j++;
                //}
                //EXPECT_EQ(j, expected.size());
            }
            //c.push_front(RowConfiguration::Entry(0, 1, false));
            //c.push_front(RowConfiguration::Entry(2, 3, true));
            //c.push_front(RowConfiguration::Entry(4, 5, false));
            //c.push_front(RowConfiguration::Entry(6, 7, true));
            //c.push_front(RowConfiguration::Entry(0, 1, false));
            //c.push_front(RowConfiguration::Entry(2, 3, true));
            //c.push_front(RowConfiguration::Entry(4, 5, false));
            //c.push_front(RowConfiguration::Entry(6, 7, true));

            //for (auto e : c)
            //{
            //    std::cout
            //        << e.GetRank()
            //        << ", " << e.GetRowCount()
            //        << ", " << e.IsPrivate()
            //        << std::endl;
            //}
        }
    }
}
#pragma warning(pop)
