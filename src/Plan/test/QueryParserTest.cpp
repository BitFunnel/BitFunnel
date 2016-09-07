#include "gtest/gtest.h"

#include <iostream> // TODO: remove.
#include <sstream>

#include "Allocator.h"
#include "BitFunnel/TermMatchNode.h"
#include "QueryParser.h"
#include "TextObjectFormatter.h"

namespace BitFunnel
{
    void VerifyQueryParser(std::string expected, std::string input, IAllocator& allocator)
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
    }


    TEST(QueryParser, Trivial)
    {
        Allocator allocator(4096);
        std::stringstream s;

        // UNIGRAM.
        VerifyQueryParser("Unigram(\"wat\", 0)", "wat", allocator);

        // STREAM:UNIGRAM.
        VerifyQueryParser("Unigram(\"wat\", 0)", "StreamsAreCurrentlyIgnored:wat", allocator);

        // (UNIGRAM).
        VerifyQueryParser("Unigram(\"wat\", 0)", "(wat)", allocator);

        // OR of two UNIGRAMs.
        VerifyQueryParser(
                          "Or {\n"
                          "  Children: [\n"
                          "    Unigram(\"foo\", 0),\n"
                          "    Unigram(\"wat\", 0)\n"
                          "  ]\n"
                          "}",
                          "wat|foo",
                          allocator);

        // OR of two UNIGRAMs with parens.
        VerifyQueryParser(
                          "Or {\n"
                          "  Children: [\n"
                          "    Unigram(\"foo\", 0),\n"
                          "    Unigram(\"wat\", 0)\n"
                          "  ]\n"
                          "}",
                          "(wat|foo)",
                          allocator);

        // NOT.
        VerifyQueryParser("Not {\n"
                          "  Child: Unigram(\"wat\", 0)\n"
                          "}"
                          , "-wat", allocator);

        // NOT with whitespace.
        VerifyQueryParser("Not {\n"
                          "  Child: Unigram(\"wat\", 0)\n"
                          "}"
                          , " \t- wat", allocator);

        // AND of 2.
        VerifyQueryParser("And {\n"
                          "  Children: [\n"
                          "    Unigram(\"foo\", 0),\n"
                          "    Unigram(\"wat\", 0)\n"
                          "  ]\n"
                          "}",
                          "wat foo", allocator);

        // AND of 2, explicit '&'.
        VerifyQueryParser("And {\n"
                          "  Children: [\n"
                          "    Unigram(\"foo\", 0),\n"
                          "    Unigram(\"wat\", 0)\n"
                          "  ]\n"
                          "}",
                          "wat&foo", allocator);

        // AND of 2, explicit '&' with whitespace.
        VerifyQueryParser("And {\n"
                          "  Children: [\n"
                          "    Unigram(\"foo\", 0),\n"
                          "    Unigram(\"wat\", 0)\n"
                          "  ]\n"
                          "}",
                          "wat\t\t&  foo", allocator);

        VerifyQueryParser("Phrase {\n"
                          "  StreamId: 0,\n"
                          "  Grams: [\n"
                          "    \"wat\",\n"
                          "    \"foo\"\n"
                          "  ]\n"
                          "}",
                          "\" wat\tfoo\"", allocator);
    }
}
