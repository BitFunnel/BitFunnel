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

#include <iostream> // TODO: remove.
#include <sstream>

#include "Allocator.h"
#include "BitFunnel/TermMatchNode.h"
#include "QueryParser.h"
#include "TextObjectFormatter.h"

namespace BitFunnel
{

    struct ExpectedAndInputs
    {
    public:
        char const * m_expected;
        char const * m_input;
    };

    const ExpectedAndInputs c_testData[] =
    {
        // UNIGRAM.
        {"Unigram(\"wat\", 0)", "wat"},

        // STREAM:UNIGRAM.
        {"Unigram(\"wat\", 0)", "StreamsAreCurrentlyIgnored:wat"},

        // (UNIGRAM)
        {"Unigram(\"wat\", 0)", "(wat)"},

        // OR of two UNIGRAMs.
        {
            "Or {\n"
            "  Children: [\n"
            "    Unigram(\"foo\", 0),\n"
            "    Unigram(\"wat\", 0)\n"
            "  ]\n"
            "}",
            "wat|foo"
         },

        // OR of two UNIGRAMs with parens.
        {
            "Or {\n"
            "  Children: [\n"
            "    Unigram(\"foo\", 0),\n"
            "    Unigram(\"wat\", 0)\n"
            "  ]\n"
            "}",
            "(wat|foo)"
        },

        // NOT.
        {
            "Not {\n"
            "  Child: Unigram(\"wat\", 0)\n"
            "}"
            , "-wat"
        },

        // AND of 2.
        {
            "And {\n"
            "  Children: [\n"
            "    Unigram(\"foo\", 0),\n"
            "    Unigram(\"wat\", 0)\n"
            "  ]\n"
            "}",
            "wat foo"
        },

        // AND of 2, explicit '&'.
        {
            "And {\n"
            "  Children: [\n"
            "    Unigram(\"foo\", 0),\n"
            "    Unigram(\"wat\", 0)\n"
            "  ]\n"
            "}",
            "wat&foo"
        },

        // AND of 2, explicit '&' with whitespace.
        {
            "And {\n"
            "  Children: [\n"
            "    Unigram(\"foo\", 0),\n"
            "    Unigram(\"wat\", 0)\n"
            "  ]\n"
            "}",
            "wat\t\t&  foo"
        }
    };




    void VerifyQueryParser(std::string const & expected, std::string const & input, IAllocator& allocator)
    {
        std::stringstream s;
        s << input;
        QueryParser parser(s, allocator);

        std::cout << "input length: " << s.str().size() << std::endl;

        auto result = parser.Parse();
        ASSERT_NE(nullptr, result);

        std::stringstream parsedOutput;
        TextObjectFormatter formatter(parsedOutput);
        result->Format(formatter);

        std::cout << "???: " << parsedOutput.str() << std::endl;
        EXPECT_EQ(expected, parsedOutput.str());
        allocator.Reset();
    }


    TEST(QueryParser, Trivial)
    {
        Allocator allocator(512);

        for (size_t i = 0; i < sizeof(c_testData) / sizeof(ExpectedAndInputs); ++i)
        {
            VerifyQueryParser(c_testData[i].m_expected, c_testData[i].m_input, allocator);
        }
    }
}
