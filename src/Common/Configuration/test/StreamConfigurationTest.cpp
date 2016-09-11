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

#include "StreamConfiguration.h"


namespace BitFunnel
{
    TEST(StreamConfiguration, AddMapping)
    {
        StreamConfiguration config;
        config.AddMapping("one", { 1, 2 });
        config.AddMapping("two", { 2, 3 });

        // First mapping should be assigned StreamId 0.
        auto id1 = config.GetStreamId("one");
        EXPECT_EQ(0, id1);

        // Second mapping should be assigned StreamId 1.
        auto id2 = config.GetStreamId("two");
        EXPECT_EQ(1, id2);

        // Document StreamId 1 maps to index stream "one".
        auto const & indexStreams1 = config.GetIndexStreamIds(1);
        EXPECT_EQ(1u, indexStreams1.size());
        EXPECT_EQ(0, indexStreams1[0]);

        // Document StreamId 2 maps to index streams "one" and "two".
        auto const & indexStreams2 = config.GetIndexStreamIds(2);
        EXPECT_EQ(2u, indexStreams2.size());
        EXPECT_EQ(0, indexStreams2[0]);
        EXPECT_EQ(1, indexStreams2[1]);

        // Document StreamId 3 maps to index stream "two".
        auto const & indexStreams3 = config.GetIndexStreamIds(3);
        EXPECT_EQ(1u, indexStreams3.size());
        EXPECT_EQ(1, indexStreams3[0]);

        // Index StreamId 0 maps to document StreamIds 1 and 2.
        auto const & documentStreams0 = config.GetDocumentStreamIds(0);
        EXPECT_EQ(2u, documentStreams0.size());
        EXPECT_EQ(1, documentStreams0[0]);
        EXPECT_EQ(2, documentStreams0[1]);

        // Index StreamId 1 maps to document StreamIds 2 and 3.
        auto const & documentStreams1 = config.GetDocumentStreamIds(1);
        EXPECT_EQ(2u, documentStreams1.size());
        EXPECT_EQ(2, documentStreams1[0]);
        EXPECT_EQ(3, documentStreams1[1]);
    }
}
