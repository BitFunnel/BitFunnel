#include "gtest/gtest.h"

#include <iostream> // TODO: remove.
#include <sstream>

#include "Allocator.h"
#include "QueryParser.h"

namespace BitFunnel
{
    TEST(QueryParser, TODO)
    {
        Allocator allocator(512);
        std::stringstream s;

        s << "wat";
        QueryParser wat(s, allocator);

        auto watResult = wat.Parse();
        std::cout << watResult << std::endl;
    }
}
