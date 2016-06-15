
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
            testHistogram.AddDocument(3);
            ASSERT_EQ(testHistogram.GetValue(3), 1u);
            testHistogram.AddDocument(3);
            ASSERT_EQ(testHistogram.GetValue(3), 2u);
            ASSERT_EQ(testHistogram.GetValue(5), 0u);
        }

        // TODO: implement and test file read/write.
    }
}
