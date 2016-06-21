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
                sprintf(persons[i].m_name, "name_%d", i);
            }
            StreamUtilities::WriteArray<TestStructPerson>(writeStream, persons,
                                                          10);
            StreamUtilities::WriteBytes(writeStream, (char *)persons,
                                        10 * sizeof(TestStructPerson));

            // Read the set of values from the stream and compare.
            int value1Read = StreamUtilities::ReadField<int32_t>(readStream);
            EXPECT_EQ(value1, value1Read);

            bool value2Read = StreamUtilities::ReadField<bool>(readStream);
            EXPECT_EQ(value2, value2Read);

            double value3Read = StreamUtilities::ReadField<double>(readStream);
            EXPECT_EQ(value3, value3Read);

            std::string strRead;
            StreamUtilities::ReadString(readStream, strRead);
            EXPECT_EQ(str, strRead);

            TestStructPerson personsRead[10];
            StreamUtilities::ReadArray<TestStructPerson>(readStream, personsRead, 10);
            for (int i = 0; i < 10; i++)
            {
                EXPECT_EQ(persons[i].m_id, personsRead[i].m_id);
                EXPECT_EQ(persons[i].m_score, personsRead[i].m_score);
                EXPECT_STREQ(persons[i].m_name, personsRead[i].m_name);
            }
            StreamUtilities::ReadBytes(readStream, (char *)personsRead, 10);
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
