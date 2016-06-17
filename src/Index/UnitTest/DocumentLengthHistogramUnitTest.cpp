
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
            ASSERT_EQ(testHistogram.GetValue(3), 0u);
            testHistogram.AddDocument(3);
            ASSERT_EQ(testHistogram.GetValue(3), 1u);
            testHistogram.AddDocument(3);
            ASSERT_EQ(testHistogram.GetValue(3), 2u);
            ASSERT_EQ(testHistogram.GetValue(5), 0u);
        }

        TEST(DocumentLengthHistogramUnitTest, StreamWrite)
        {
            DocumentLengthHistogram testHistogram;
            testHistogram.AddDocument(3);
            std::stringstream stream;
            testHistogram.Write(stream);
            ASSERT_EQ(" ", stream.str());
        }

        // TODO: implement and test file read/write.
    }
}
