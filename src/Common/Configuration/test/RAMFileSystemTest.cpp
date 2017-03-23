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

#include "RAMFileSystem.h"


namespace BitFunnel
{
    TEST(RAMFileSystem, WriteThenRead)
    {
        RAMFileSystem files;

        // Write to first file.
        char const * expected1 = "Contents of file 1.";
        char const * name1 = "name1";
        auto output1 = files.OpenForWrite(name1);
        *output1 << expected1 << std::endl;

        // Write to second file.
        char const * expected2 = "Contents of file 2.";
        char const * name2 = "/temp/name2.txt";
        auto output2 = files.OpenForWrite(name2);
        *output2 << expected2 << std::endl;

        // Read from first file.
        auto input1 = files.OpenForRead(name1);
        std::string observed1;
        std::getline(*input1, observed1);
        EXPECT_STREQ(expected1, observed1.c_str());

        // Read from second file.
        auto input2 = files.OpenForRead(name2);
        std::string observed2;
        std::getline(*input2, observed2);
        EXPECT_STREQ(expected2, observed2.c_str());
    }


    TEST(RAMFileSystem, Reopen)
    {
        RAMFileSystem files;

        char const * expected1 = "Contents of file 1.";
        char const * expected2 = "Totally different.";
        char const * name = "name1";

        {
            // Write to first file.
            auto output = files.OpenForWrite(name);
            EXPECT_EQ(0, output->tellp());
            *output << expected1 << std::endl;

            // Read from first file.
            auto input = files.OpenForRead(name);
            std::string observed;
            std::getline(*input, observed);
            EXPECT_STREQ(expected1, observed.c_str());

            // Let first file close.
        }

        {
            // Write a different string to the first file.
            auto output = files.OpenForWrite(name);

            // Verify that we're at the beginning of the stream.
            EXPECT_EQ(0, output->tellp());

            // Write to stream.
            *output << expected2 << std::endl;


            // Reopen first file.
            auto input = files.OpenForRead(name);

            // Verify that we're at the beginning of the stream.
            EXPECT_EQ(0, input->tellg());

            std::string observed;
            std::getline(*input, observed);
            EXPECT_STREQ(expected2, observed.c_str());
        }

        {
            // Reopen first file.
            auto input = files.OpenForRead(name);

            // Verify that we're at the beginning of the stream.
            EXPECT_EQ(0, input->tellg());

            std::string observed;
            std::getline(*input, observed);
            EXPECT_STREQ(expected2, observed.c_str());
        }
    }
}
