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


#include <sstream>

#include "gtest/gtest.h"

#include "BitFunnel/Utilities/Version.h"


namespace BitFunnel
{
    namespace VersionUnitTest
    {
        TEST(Manual, Trivial)
        {
            Version version(1, 1, 0);

            std::stringstream output;
            version.Write(output);

            Version version2(output);

            EXPECT_EQ(version.VersionMajor(), version2.VersionMajor());
            EXPECT_EQ(version.VersionMiddle(), version2.VersionMiddle());
            EXPECT_EQ(version.VersionMinor(), version2.VersionMinor());

            Version version3(1, 1, 1);
            EXPECT_TRUE(version.IsCompatibleWith(version3));

            Version version4(1, 2, 0);
            EXPECT_TRUE(!version.IsCompatibleWith(version4));

            Version version5(2, 1, 0);
            EXPECT_TRUE(!version.IsCompatibleWith(version5));

            Version version6 = version.IncrementMajor();
            EXPECT_EQ(version6.VersionMajor(), 2);
            EXPECT_EQ(version6.VersionMiddle(), 0);
            EXPECT_EQ(version6.VersionMinor(), 0);

            Version version7 = version.IncrementMiddle();
            EXPECT_EQ(version7.VersionMajor(), 1);
            EXPECT_EQ(version7.VersionMiddle(), 2);
            EXPECT_EQ(version7.VersionMinor(), 0);

            Version version8 = version.IncrementMinor();
            EXPECT_EQ(version8.VersionMajor(), 1);
            EXPECT_EQ(version8.VersionMiddle(), 1);
            EXPECT_EQ(version8.VersionMinor(), 1);
        }
    }
}
