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

#include "CsvExtract.h"
#include "LoggerInterfaces/Check.h"


namespace BitFunnel
{
    TEST(CsvExtractTest, ExtractTwoFromFour)
    {
        std::stringstream input;
        input
            << "a,b,c,d,e\n"
            << "1,2,3,4,5\n"
            << "p,q,r,s,t\n";

        std::vector<std::string> columns;
        columns.push_back("d");
        columns.push_back("b");

        std::string expected =
            "d,b\n"
            "4,2\n"
            "s,q\n";

        std::stringstream output;

        CsvExtract::Filter(input, output, columns);

        ASSERT_EQ(expected, output.str());
    }


    TEST(CsvExtractTest, ExtractFourFromFour)
    {
        std::stringstream input;
        input
            << "a,b,c,d,e\n"
            << "1,2,3,4,5\n"
            << "p,q,r,s,t\n";

        std::vector<std::string> columns;
        columns.push_back("e");
        columns.push_back("d");
        columns.push_back("c");
        columns.push_back("b");
        columns.push_back("a");

        std::string expected =
            "e,d,c,b,a\n"
            "5,4,3,2,1\n"
            "t,s,r,q,p\n";

        std::stringstream output;

        CsvExtract::Filter(input, output, columns);

        ASSERT_EQ(expected, output.str());
    }


    TEST(CsvExtractTest, ExtractZeroFromFour)
    {
        std::stringstream input;
        input
            << "a,b,c,d,e\n"
            << "1,2,3,4,5\n"
            << "p,q,r,s,t\n";

        std::vector<std::string> columns;

        std::stringstream output;

        ASSERT_THROW(CsvExtract::Filter(input, output, columns), Logging::CheckException);
    }


    TEST(CsvExtractTest, ColumnNotFound)
    {
        std::stringstream input;
        input
            << "a,b,c,d,e\n"
            << "1,2,3,4,5\n"
            << "p,q,r,s,t\n";

        std::vector<std::string> columns;
        columns.push_back("MISSING");

        std::stringstream output;

        ASSERT_THROW(CsvExtract::Filter(input, output, columns), Logging::CheckException);
    }
}
