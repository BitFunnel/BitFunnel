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

#pragma once

#include "AnswerResponseUnitTestBase.h"

namespace CmdLine
{
    class CmdLineParser;

    class CmdLineParserUnitTest : public AnswerResponseUnitTestBase
    {
    public:
        CmdLineParserUnitTest(bool createBaseline);
        ~CmdLineParserUnitTest();

        static bool RunAllCases(bool createBaseline);

    private:
        void InitializeCases();
        void RunOneCase(const std::string& input, const std::string& expected);

        void TestRequiredParameters(std::ostream& output, int argc, char const* const* argv);
        void TestOptionalParameters(std::ostream& output, int argc, char const* const* argv);
        void TestConstraints(std::ostream& output, int argc, char const* const* argv);
        void TestComplexParameters(std::ostream& output, int argc, char const* const* argv);

        static void ParseCommandLine(std::string cmd, int& argc, char**& argv);
    };
}
