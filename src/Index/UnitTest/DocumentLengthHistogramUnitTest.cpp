
#include "DocumentLengthHistogram.h"
#include "gtest/gtest.h"


namespace BitFunnel
{
    namespace DocumentLengthHistogramUnitTest
    {

        //*********************************************************************
        TEST(DocumentLengthHistogramUnitTest, Trivial)
        {
            DocumentLengthHistogram testHistogram;

            ASSERT_ANY_THROW(testHistogram.AddDocument(0));

            ASSERT_EQ(testHistogram.GetValue(3), 0u);
            ASSERT_EQ(testHistogram.GetValue(5), 0u);

            testHistogram.AddDocument(3);
            ASSERT_EQ(testHistogram.GetValue(3), 1u);

            testHistogram.AddDocument(3);
            ASSERT_EQ(testHistogram.GetValue(3), 2u);

            ASSERT_EQ(testHistogram.GetValue(5), 0u);
            testHistogram.AddDocument(5);
            ASSERT_EQ(testHistogram.GetValue(5), 1u);

            std::stringstream stream;
            testHistogram.Write(stream);
            ASSERT_EQ("Postings,Count\n3,2\n5,1\n", stream.str());
        }

        // TODO: implement and test file read/write.
    }
}
