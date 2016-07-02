#include <memory>

#include "gtest/gtest.h"

// #include "BitFunnel/BitFunnelTypes.h"    // For DocId.
#include "BitFunnel/Index/DocumentHandle.h"  // For DocId.
#include "DocumentDataSchema.h"

namespace BitFunnel
{
    namespace DocumentDataSchemaUnitTest
    {
        TEST(BasicTest, Trivial)
        {
            DocumentDataSchema schema;
            // static const unsigned s_sizeOfDocId(sizeof(DocId));

            EXPECT_TRUE(schema.GetFixedSizeBlobSizes().empty());
            EXPECT_EQ(schema.GetVariableSizeBlobCount(), 0u);

            const FixedSizeBlobId fixedBlob0 = schema.RegisterFixedSizeBlob(10);

            // fixedBlob0 is of size 10.
            {
                std::vector<unsigned> const & sizes = schema.GetFixedSizeBlobSizes();
                EXPECT_EQ(sizes.size(), 1u);
                EXPECT_EQ(sizes[static_cast<unsigned>(fixedBlob0)], 10u);
            }

            /* const VariableSizeBlobId variableBlob0 = */ schema.RegisterVariableSizeBlob();
            EXPECT_EQ(schema.GetVariableSizeBlobCount(), 1u);

            // Register another fixed size blob.
            const FixedSizeBlobId fixedBlob1 = schema.RegisterFixedSizeBlob(5);
            EXPECT_EQ(schema.GetVariableSizeBlobCount(), 1u);
            {
                std::vector<unsigned> const & sizes = schema.GetFixedSizeBlobSizes();

                EXPECT_EQ(sizes.size(), 2u);
                EXPECT_EQ(sizes[static_cast<unsigned>(fixedBlob0)], 10u);
                EXPECT_EQ(sizes[static_cast<unsigned>(fixedBlob1)], 5u);
            }

            // Register another variable size blob.
            /* const VariableSizeBlobId variableBlob1 = */ schema.RegisterVariableSizeBlob();
            EXPECT_EQ(schema.GetVariableSizeBlobCount(), 2u);
        }
    }
}
