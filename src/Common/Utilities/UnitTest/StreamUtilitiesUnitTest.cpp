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

#include "BitFunnel/Utilities/StandardInputStream.h"
#include "BitFunnel/Utilities/StreamUtilities.h"

namespace BitFunnel
{
    namespace StreamUtilitiesUnitTest
    {

        struct TestStructPerson
        {
            int m_id;
            double m_score;
            char m_name[32];
        };


        template <typename T>
        void TestInternal(std::stringstream& writeStream, T& readStream)
        {
            // Write a set of values to a stream.
            int value1 = 44;
            StreamUtilities::WriteField<int32_t>(writeStream, value1);
            bool value2 = true;
            StreamUtilities::WriteField<bool>(writeStream, value2);
            double value3 = 0.34567;
            StreamUtilities::WriteField<double>(writeStream, value3);

            std::string str = "abcdef";
            StreamUtilities::WriteString(writeStream, str);

            TestStructPerson persons[10];
            for (int i = 0; i < 10; i++)
            {
                persons[i].m_id = i;
                persons[i].m_score = 100 - i;
                #ifndef _MSC_VER
                sprintf(persons[i].m_name, "name_%d", i);
                #else
                sprintf_s(persons[i].m_name, 32, "name_%d", i);
                #endif
            }
            StreamUtilities::WriteArray<TestStructPerson>(writeStream, persons,
                                                          10);
            StreamUtilities::WriteBytes(writeStream,
                                        reinterpret_cast<char *>(persons),
                                        10 * sizeof(TestStructPerson));

            // Read the set of values from the stream and compare.
            int value1Read = StreamUtilities::ReadField<int32_t>(readStream);
            EXPECT_EQ(value1, value1Read);

            bool value2Read = StreamUtilities::ReadField<bool>(readStream);
            EXPECT_EQ(value2, value2Read);

            double value3Read = StreamUtilities::
                ReadField<double>(readStream);
            EXPECT_EQ(value3, value3Read);

            std::string strRead;
            StreamUtilities::ReadString(readStream, strRead);
            EXPECT_EQ(str, strRead);

            TestStructPerson personsRead[10];
            StreamUtilities::ReadArray<TestStructPerson>(readStream,
                                                         personsRead, 10);
            for (int i = 0; i < 10; i++)
            {
                EXPECT_EQ(persons[i].m_id, personsRead[i].m_id);
                EXPECT_EQ(persons[i].m_score, personsRead[i].m_score);
                EXPECT_STREQ(persons[i].m_name, personsRead[i].m_name);
            }
            StreamUtilities::ReadBytes(readStream,
                                       reinterpret_cast<char *>(personsRead),
                                       10);
            for (int i = 0; i < 10; i++)
            {
                EXPECT_EQ(persons[i].m_id, personsRead[i].m_id);
                EXPECT_EQ(persons[i].m_score, personsRead[i].m_score);
                EXPECT_STREQ(persons[i].m_name, personsRead[i].m_name);
            }
        }


        TEST(RoundTripIInputStream, Trivial)
        {
            std::stringstream stream;
            StandardInputStream stdStream(stream);

            TestInternal(stream, stdStream);
        }


        TEST(RoundTripStdStream, Trivial)
        {
            std::stringstream stream;

            TestInternal(stream, stream);
        }
    }
}
