
#include <vector>

#include "gtest/gtest.h"

#include "ChunkEnumerator.h"
#include "IIndex.h"
// #include "DocumentLengthHistogram.h"

namespace BitFunnel
{
    namespace DocumentLengthHistogramUnitTest
    {
        static std::vector<std::string> const filePaths = {
            "Data/1.chunk",
            "Data/2.chunk",
        };

        //*********************************************************************
        // TEST(DocumentLengthHistogramUnitTest, Trivial)
        // {
        //     DocumentLengthHistogram testHistogram;
        //     testHistogram.AddDocument(3);
        //     ASSERT_EQ(testHistogram.GetValue(3), 1u);
        //     testHistogram.AddDocument(3);
        //     ASSERT_EQ(testHistogram.GetValue(3), 2u);
        //     ASSERT_EQ(testHistogram.GetValue(5), 0u);
        // }

        //*********************************************************************
        TEST(EndToEndTest, Trivial)
        {
            IIndex index;
            ChunkEnumerator chunkEnumerator(filePaths, index, 2);
        }

        // TODO: implement and test file read/write.
    }
}
