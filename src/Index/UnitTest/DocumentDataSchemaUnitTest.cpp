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


#include <memory>

#include "gtest/gtest.h"

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
