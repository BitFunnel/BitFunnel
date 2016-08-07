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

#include "ShardDefinition.h"


namespace BitFunnel
{
    namespace ShardDefinitionTest
    {
        TEST(ShardDefinition, AddShards)
        {
            ShardDefinition s;
            s.AddShard(500);
            s.AddShard(400);
            s.AddShard(600);
            s.AddShard(450);

            EXPECT_EQ(s.GetShardCount(), 5u);

            for (ShardId i = 0; i < s.GetShardCount() - 2; ++i)
            {
                EXPECT_LT(s.GetMaxPostingCount(i), s.GetMaxPostingCount(i + 1));
            }
        }


        TEST(ShardDefinition, AddDuplicate)
        {
            // ShardDefinition should throw an exception if the same shard is added twice.
            // Also if max is zero.
            // TODO.
        }


        TEST(ShardDefinition, GetShard)
        {
            ShardDefinition s;
            s.AddShard(400);
            s.AddShard(500);
            s.AddShard(600);
            s.AddShard(750);

            EXPECT_EQ(s.GetShard(0), 0u);
            EXPECT_EQ(s.GetShard(10), 0u);
            EXPECT_EQ(s.GetShard(400), 0u);
            EXPECT_EQ(s.GetShard(401), 1u);
            EXPECT_EQ(s.GetShard(750), 3u);
            EXPECT_EQ(s.GetShard(751), 4u);
            EXPECT_EQ(s.GetShard(10000), 4u);
        }


        // TEST(ShardDefinition, RoundTrip)
        // {
        //     ShardDefinition s1;
        //     s1.AddShard(500);
        //     s1.AddShard(400);
        //     s1.AddShard(600);
        //     s1.AddShard(450);

        //     std::stringstream stream;
        //     s1.Write(stream);

        //     // Ensure the test fails if s1 isn't loaded correctly.
        //     // Perhaps one really wants to ensure that characters were written to stream.
        //     // Want to guard against writing nothing and the passing the test when nothing is read.
        //     EXPECT_EQ(s1.GetShardCount(), 5u);


        //     ShardDefinition s2(stream);

        //     EXPECT_EQ(s1.GetShardCount(), s2.GetShardCount());
        //     for (ShardId i = 0; i < s1.GetShardCount(); ++i)
        //     {
        //         EXPECT_EQ(s1.GetMaxPostingCount(i), s2.GetMaxPostingCount(i));
        //     }
        // }
    }
}
