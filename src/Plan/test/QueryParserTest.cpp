#include "gtest/gtest.h"

#include <iostream> // TODO: remove.
#include <sstream>

#include "Allocator.h"
#include "BitFunnel/TermMatchNode.h"
#include "QueryParser.h"
#include "TextObjectFormatter.h"

namespace BitFunnel
{
    void TodoFnName()
    {

    }


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


    TEST(QueryParser, TODO)
    {
        Allocator allocator(512);
        std::stringstream s;

        VerifyQueryParser("Unigram(\"wat\", 0)", "wat notinsideunigram", allocator);
        VerifyQueryParser("Unigram(\"wat\", 0)", "wat|notinsideunigram", allocator);
        VerifyQueryParser("Unigram(\"wat\", 0)", "wat", allocator);
    }
}
