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


#include "DocumentHistogramBuilder.h"
#include "gtest/gtest.h"


namespace BitFunnel
{
    //*********************************************************************
    TEST(DocumentHistogramBuilder, Basic)
    {
        DocumentHistogramBuilder testHistogram;

        // NOTE: There are valid documents with 0 postings. This is a
        // specific design decision and the test below captures this
        // behaviour.
        ASSERT_EQ(testHistogram.GetValue(0), 0u);
        testHistogram.AddDocument(0);
        ASSERT_EQ(testHistogram.GetValue(0), 1u);

        ASSERT_EQ(testHistogram.GetValue(3), 0u);
        ASSERT_EQ(testHistogram.GetValue(5), 0u);

        testHistogram.AddDocument(3);
        ASSERT_EQ(testHistogram.GetValue(3), 1u);

        testHistogram.AddDocument(3);
        ASSERT_EQ(testHistogram.GetValue(3), 2u);

        ASSERT_EQ(testHistogram.GetValue(5), 0u);
        testHistogram.AddDocument(5);
        ASSERT_EQ(testHistogram.GetValue(5), 1u);

        std::stringstream stream;
        testHistogram.Write(stream);
        ASSERT_EQ("Postings,Count\n0,1\n3,2\n5,1\n", stream.str());
    }

        // TODO: Implement and test file read/write.
}
