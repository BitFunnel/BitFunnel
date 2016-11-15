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

#include "BitFunnel/Utilities/Allocator.h"
#include "BitFunnel/Utilities/StringBuilder.h"


namespace BitFunnel
{
    TEST(StringBuilder, Constructor)
    {
        Allocator allocator(4096);

        std::string anyString("hello");

        // Construct from std::string.
        StringBuilder s1(allocator, anyString);
        EXPECT_STREQ(anyString.c_str(), s1);

        // Construct from char const *.
        StringBuilder s2(allocator, anyString.c_str());
        EXPECT_STREQ(anyString.c_str(), s2);

        // Construct empty.
        StringBuilder s3(allocator);
        EXPECT_STREQ("", s3);
    }


    TEST(StringBuilder, PushBack)
    {
        Allocator allocator(4096);

        std::string src("a string to copy.");

        StringBuilder s(allocator, 1ull);

        for (size_t i = 0; i < src.size(); ++i)
        {
            s.push_back(src[i]);
            EXPECT_EQ(i + 1, strlen(static_cast<char *>(s)));
            EXPECT_EQ(0ull, src.find(static_cast<char*>(s)));
        }
    }


    // Forces the StringBuilder to resize its buffer at least once.
    // Verifies that the buffer pointer before the resize is not equal to the
    // buffer pointer after the resize.
    // NOTE: This test is coupled to the implementation in that it assumes
    // that s(allocator) creates a buffer with capacity <= c_initialCapacity.
    TEST(StringBuilder, Resize)
    {
        Allocator allocator(4096);
        std::string data16("0123456789abcdef");

        StringBuilder s(allocator);

        char const* beforeResize = static_cast<char*>(s);

        size_t count = 0;
        while (count <= StringBuilder::c_initialCapacity)
        {
            s.append(data16);
            count += data16.size();
        }

        char const* afterResize = static_cast<char*>(s);

        EXPECT_NE(beforeResize, afterResize);
    }


    TEST(StringBuilder, AppendCString)
    {
        Allocator allocator(4096);

        std::string src("a string to copy.");

        StringBuilder s(allocator, 1ull);
        std::string expected;

        for (size_t i = 0; i < src.size(); ++i)
        {
            s.append(src.c_str() + i);
            expected.append(src.c_str() + i);
            EXPECT_STREQ(expected.c_str(), s);
        }
    }


    TEST(StringBuilder, AppendStdString)
    {
        Allocator allocator(4096);

        std::string src("a string to copy.");

        StringBuilder s(allocator, 1ull);
        std::string expected;

        for (size_t i = 0; i < src.size(); ++i)
        {
            s.append(std::string(src.c_str() + i));
            expected.append(src.c_str() + i);
            EXPECT_STREQ(expected.c_str(), s);
        }
    }
}
