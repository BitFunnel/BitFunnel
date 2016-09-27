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

#include "gtest/gtest.h"

#include "LoggerInterfaces/Check.h"


namespace Logging
{
    TEST(Logging, CheckOpPass)
    {
        int i1 = 1;
        int i2 = 2;
        unsigned u1 = 3;
        unsigned u2 = 4;
        float f1 = 5.5f;
        float f2 = 6.6f;
        int* p1 = &i1;
        int* p2 = &i2;
        char const * message = "message";

        CHECK_EQ(i1, i1) << message;
        CHECK_EQ(u1, u1) << message;
        CHECK_EQ(f1, f1) << message;
        CHECK_EQ(p1, p1) << message;

        CHECK_NE(i1, i2) << message;
        CHECK_NE(u1, u2) << message;
        CHECK_NE(f1, f2) << message;
        CHECK_NE(p1, p2) << message;

        CHECK_LT(i1, i2) << message;
        CHECK_LT(u1, u2) << message;
        CHECK_LT(f1, f2) << message;

        CHECK_LE(i1, i1) << message;
        CHECK_LE(u1, u2) << message;
        CHECK_LE(f1, f1) << message;

        CHECK_GT(i2, i1) << message;
        CHECK_GT(u2, u1) << message;
        CHECK_GT(f2, f1) << message;

        CHECK_GE(i1, i1) << message;
        CHECK_GE(u2, u1) << message;
        CHECK_GE(f1, f1) << message;
    }


    TEST(Logging, CheckOpFail)
    {
        int i1 = 1;
        int i2 = 2;
        unsigned u1 = 3;
        unsigned u2 = 4;
        float f1 = 5.5f;
        float f2 = 6.6f;
        int* p1 = &i1;
        int* p2 = &i2;
        char const * message = "message";

        EXPECT_THROW(CHECK_NE(i1, i1) << message, Logging::CheckException);
        EXPECT_THROW(CHECK_NE(u1, u1) << message, Logging::CheckException);
        EXPECT_THROW(CHECK_NE(f1, f1) << message, Logging::CheckException);
        EXPECT_THROW(CHECK_NE(p1, p1) << message, Logging::CheckException);

        EXPECT_THROW(CHECK_EQ(i1, i2) << message, Logging::CheckException);
        EXPECT_THROW(CHECK_EQ(u1, u2) << message, Logging::CheckException);
        EXPECT_THROW(CHECK_EQ(f1, f2) << message, Logging::CheckException);
        EXPECT_THROW(CHECK_EQ(p1, p2) << message, Logging::CheckException);

        EXPECT_THROW(CHECK_GT(i1, i2) << message, Logging::CheckException);
        EXPECT_THROW(CHECK_GT(u1, u2) << message, Logging::CheckException);
        EXPECT_THROW(CHECK_GT(f1, f2) << message, Logging::CheckException);

        EXPECT_THROW(CHECK_GT(i1, i1) << message, Logging::CheckException);
        EXPECT_THROW(CHECK_GT(u1, u2) << message, Logging::CheckException);
        EXPECT_THROW(CHECK_GT(f1, f1) << message, Logging::CheckException);

        EXPECT_THROW(CHECK_LT(i2, i1) << message, Logging::CheckException);
        EXPECT_THROW(CHECK_LT(u2, u1) << message, Logging::CheckException);
        EXPECT_THROW(CHECK_LT(f2, f1) << message, Logging::CheckException);

        EXPECT_THROW(CHECK_LT(i1, i1) << message, Logging::CheckException);
        EXPECT_THROW(CHECK_LT(u2, u1) << message, Logging::CheckException);
        EXPECT_THROW(CHECK_LT(f1, f1) << message, Logging::CheckException);
    }


    TEST(Logging, CheckBool)
    {
        bool b1 = true;
        bool b2 = false;
        char const * message = "message";

        CHECK_TRUE(b1) << message;
        CHECK_FALSE(b2) << message;

        EXPECT_THROW(CHECK_TRUE(b2) << message, Logging::CheckException);
        EXPECT_THROW(CHECK_FALSE(b1) << message, Logging::CheckException);
    }


    TEST(Logging, Message)
    {
        // Check general message.
        try
        {
            int i1 = 1;
            int i2 = 2;
            char const * message = "message";
            CHECK_EQ(i1, i2) << message;
        }
        catch (Logging::CheckException e)
        {
            EXPECT_STREQ(e.GetMessage().c_str(),
                         "Expected: (i1) EQ (i2), actual: (1) vs (2)\nmessage");
        }


        // Special case formatting for bool binary operator.
        try
        {
            bool b1 = false;
            bool b2 = true;
            char const * message = "message";
            CHECK_EQ(b1, b2) << message;
        }
        catch (Logging::CheckException e)
        {
            EXPECT_STREQ(e.GetMessage().c_str(),
                         "Expected: (b1) EQ (b2), actual: (false) vs (true)\nmessage");
        }


        // Formatting for CHECK_TRUE().
        try
        {
            bool b1 = false;
            char const * message = "message";
            CHECK_TRUE(b1) << message;
        }
        catch (Logging::CheckException e)
        {
            EXPECT_STREQ(e.GetMessage().c_str(),
                         "Value of: b1\n  Actual: false\n  Expected: true\nmessage");
        }


        // Special case formatting for pointers.
        {
            // NOTE: Need to construct the pointers directly with
            // reinterpret_cast to ensure that the test passes in the Release
            // build. Simply taking the addresses of local variables is
            // unreliable in the release build. Original code was
            //   int i1 = 1;
            //   int i2 = 2;
            //   CHECK_EQU(&i1, &i2);
            // In the release build, i1 would often have different values on
            // its first and second references.
            int *ptr1 = reinterpret_cast<int*>(1);
            int *ptr2 = reinterpret_cast<int*>(2);
            try
            {
                char const * message = "message";
                CHECK_EQ(ptr1, ptr2) << message;
            }
            catch (Logging::CheckException e)
            {
                std::stringstream s;
                s << "Expected: (ptr1) EQ (ptr2), actual: ("
                    << std::hex << ptr1 << ") vs (" << std::hex << ptr2 << ")\nmessage";
                EXPECT_STREQ(e.GetMessage().c_str(), s.str().c_str());
            }
        }


        // Special case formatting for nullptr.
        {
            int* ptr1 = reinterpret_cast<int*>(1);
            int* ptr2 = nullptr;
            try
            {
                char const * message = "message";
                CHECK_EQ(ptr1, ptr2) << message;
            }
            catch (Logging::CheckException e)
            {
                std::stringstream s;
                s << "Expected: (ptr1) EQ (ptr2), actual: ("
                    << std::hex << ptr1 << ") vs ((nullptr))\nmessage";
                EXPECT_STREQ(e.GetMessage().c_str(), s.str().c_str());
            }
        }
    }
}
