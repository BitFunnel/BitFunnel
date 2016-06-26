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

#include "BitFunnel/Utilities/FileHeader.h"
#include "BitFunnel/Utilities/Version.h"

namespace BitFunnel
{
    namespace FileHeaderUnitTest
    {
        TEST(Roundtrip, Trivial)
        {
            Version version(1, 1, 0);
            FileHeader fileHeader(version, "Hello World");
            std::stringstream stream;
            fileHeader.Write(stream);
            FileHeader fileHeader2(stream);
            EXPECT_EQ(fileHeader2.GetVersion().VersionMajor(),
                      fileHeader.GetVersion().VersionMajor());
            EXPECT_EQ(fileHeader2.GetVersion().VersionMiddle(),
                      fileHeader.GetVersion().VersionMiddle());
            EXPECT_EQ(fileHeader2.GetVersion().VersionMinor(),
                      fileHeader.GetVersion().VersionMinor());
            EXPECT_EQ(fileHeader2.TimeStamp(),
                      fileHeader.TimeStamp());
            EXPECT_EQ(fileHeader2.UserData(),
                      fileHeader.UserData());

            std::stringstream stream2;
            fileHeader2.Write(stream2);

            EXPECT_EQ(stream.str(), stream2.str());

<<<<<<< 81f1d08980c1aab68dad9be965d573d5373e001e
=======
            EXPECT_EQ(fileHeader2.GetVersion().VersionMajor(),
                      fileHeader.GetVersion().VersionMajor());
            EXPECT_EQ(fileHeader2.GetVersion().VersionMiddle(),
                      fileHeader.GetVersion().VersionMiddle());
            EXPECT_EQ(fileHeader2.GetVersion().VersionMinor(),
                      fileHeader.GetVersion().VersionMinor());
            // TODO: figure out what to do with this time API.
            // EXPECT_EQ(fileHeader2.TimeStamp(),
            //        fileHeader.TimeStamp());
            EXPECT_EQ(fileHeader2.UserData(),
                      fileHeader.UserData());

            std::stringstream stream2;
            fileHeader2.Write(stream2);

            EXPECT_EQ(stream.str(), stream2.str());

>>>>>>> Port FileHeader.
        }
    }
}
