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

#include "CmdLineParserUnitTest.h"
#include "CmdLineParser/CmdLineParser.h"
#include "gtest/gtest.h"


using namespace CmdLine;


bool g_createBaseline = false;


TEST(CmdLineParser, AllTests)
{
    CmdLineParserUnitTest test(g_createBaseline);
    test.RunAllCases(g_createBaseline);
}


int main(int argc, char* argv[])
{
    int success = 0;

    CmdLineParser parser("CmdLineParser Unit Test",
                         "Runs unit tests for the CmdLineParser.Library package.");

    OptionalParameterList createBaseline("createBaseline",
                                         "Outputs source code for new CmdLineParserCases.cpp that captures the current output of all of the test cases.");

    parser.AddParameter(createBaseline);

    if (!parser.TryParse(std::cout, argc, argv))
    {
        parser.Usage(std::cout, argv[0]); // TODO: figure out correct dummy to pass in.
    }
    else
    {
        // Need to put local createBaseline into global so that it is
        // accessible from GTest TEST(CmdLineParser, AllTests).
        g_createBaseline = createBaseline;

        int dummyArgc = 1;
        char* dummyArgv[] = { const_cast<char*>("CmdLineParserUnitTest.exe") };

        ::testing::InitGoogleTest(&dummyArgc, dummyArgv);
        success = RUN_ALL_TESTS();
    }

    return success;
}
