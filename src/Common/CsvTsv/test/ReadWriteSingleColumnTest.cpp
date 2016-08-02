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

#include <iostream>
#include <vector>

#include "CsvTsv/Csv.h"
#include "gtest/gtest.h"


namespace CsvTsv
{
    template <typename T>
    static bool TestRoundTrip(const char* input)
    {
        std::stringstream errorStream;

        std::stringstream inputStream(input);
        std::vector<T> values;

        ReadCsvColumn(inputStream, values);

        std::stringstream outputStream;
        WriteCsvColumn(outputStream, values);

        bool success = (inputStream.str().compare(outputStream.str()) == 0);

        return success;
    }


    TEST(CsvTsv, ReadWriteSingleColumnTest)
    {
        ASSERT_TRUE(TestRoundTrip<int>("1\n-2\n3\n-4\n"));
        ASSERT_TRUE(TestRoundTrip<unsigned int>("1\n2\n3\n4\n"));
        ASSERT_TRUE(TestRoundTrip<std::string>("one\ntwo\nthree\nfour\n"));
        ASSERT_TRUE(TestRoundTrip<double>("1.1\n2.1\n3\n4\n"));
        ASSERT_TRUE(TestRoundTrip<bool>("TRUE\nFALSE\nTRUE\nFALSE\n"));
        ASSERT_TRUE(TestRoundTrip<long long>("1\n-2\n3\n-4\n"));
        ASSERT_TRUE(TestRoundTrip<unsigned long long>("1\n2\n3\n4\n"));
    }
}
