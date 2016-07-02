#include "stdafx.h"

#include <memory>

#include "BitFunnel/BitFunnelTypes.h"    // For DocId.
#include "DocumentDataSchema.h"
#include "SuiteCpp/UnitTest.h"

namespace BitFunnel
{
    namespace DocumentDataSchemaUnitTest
    {
        TestCase(BasicTest)
        {
            DocumentDataSchema schema;
            static const unsigned s_sizeOfDocId(sizeof(DocId));

            TestAssert(schema.GetFixedSizeBlobSizes().empty());
            TestEqual(schema.GetVariableSizeBlobCount(), 0u);

            const FixedSizeBlobId fixedBlob0 = schema.RegisterFixedSizeBlob(10);

            // fixedBlob0 is of size 10.
            {
                std::vector<unsigned> const & sizes = schema.GetFixedSizeBlobSizes();
                TestEqual(sizes.size(), 1u);
                TestEqual(sizes[static_cast<unsigned>(fixedBlob0)], 10u);
            }

            /* const VariableSizeBlobId variableBlob0 = */ schema.RegisterVariableSizeBlob();
            TestEqual(schema.GetVariableSizeBlobCount(), 1u);

            // Register another fixed size blob.
            const FixedSizeBlobId fixedBlob1 = schema.RegisterFixedSizeBlob(5);
            TestEqual(schema.GetVariableSizeBlobCount(), 1u);
            {
                std::vector<unsigned> const & sizes = schema.GetFixedSizeBlobSizes();

                TestEqual(sizes.size(), 2u);
                TestEqual(sizes[static_cast<unsigned>(fixedBlob0)], 10u);
                TestEqual(sizes[static_cast<unsigned>(fixedBlob1)], 5u);
            }

            // Register another variable size blob.
            /* const VariableSizeBlobId variableBlob1 = */ schema.RegisterVariableSizeBlob();
            TestEqual(schema.GetVariableSizeBlobCount(), 2u);
        }
    }
}