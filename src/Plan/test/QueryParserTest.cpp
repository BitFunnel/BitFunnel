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

    TEST(QueryParser, TODO)
    {
        Allocator allocator(512);
        std::stringstream s;

        s << "wat";
        QueryParser wat(s, allocator);

        auto watResult = wat.Parse();
        ASSERT_NE(nullptr, watResult);

        std::stringstream output;
        TextObjectFormatter formatter(output);
        watResult->Format(formatter);

        std::cout << output.str() << std::endl;
    }
}
