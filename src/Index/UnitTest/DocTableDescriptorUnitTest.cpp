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


#include "gtest/gtest.h"

#include "AlignedBuffer.h"
#include "DocTableDescriptor.h"
#include "DocumentDataSchema.h"
#include "Rounding.h"


namespace BitFunnel
{
    namespace DocTableDescriptorUnitTest
    {
        // TODO: figure out what to do with this.
        static const size_t c_docTableByteAlignment = 8;

        struct FixedSizeBlob0
        {
            unsigned m_field1;
            float m_field2;
        };


        static bool operator==(FixedSizeBlob0 const & first, FixedSizeBlob0 const & second)
        {
            return first.m_field1 == second.m_field1
                   && first.m_field2 == second.m_field2;
        }


        struct FixedSizeBlob1
        {
            double m_field1;
            uint64_t m_field2;
        };


        // Helper function which allocates blobs and verifies they are
        // being returned properly from the DocTable API.
        void TestAllocateBlob(DocTableDescriptor& docTable,
                              void* buffer,
                              DocIndex index,
                              VariableSizeBlobId blob,
                              size_t blobSize,
                              char fill)
        {
            void* blobData = docTable.GetVariableSizeBlob(buffer, index, blob);

            EXPECT_EQ(blobData, nullptr);

            blobData = docTable.AllocateVariableSizeBlob(buffer, index, blob, blobSize);

            EXPECT_NE(blobData, nullptr);

            void* blobDataTest = docTable.GetVariableSizeBlob(buffer, index, blob);

            EXPECT_EQ(blobData, blobDataTest);

            memset(blobData, fill, blobSize);
        }


        TEST(BasicTest, Trivial)
        {
            static const DocIndex c_capacity = 1000;
            static const ptrdiff_t c_anyDocTableBufferOffset
                = RoundUp(1234, c_docTableByteAlignment);

            DocumentDataSchema schema;
            const VariableSizeBlobId variableBlob0 = schema.RegisterVariableSizeBlob();
            const VariableSizeBlobId variableBlob1 = schema.RegisterVariableSizeBlob();

            const FixedSizeBlobId fixedSizeBlob0 = schema.RegisterFixedSizeBlob(sizeof(FixedSizeBlob0));
            const FixedSizeBlobId fixedSizeBlob1 = schema.RegisterFixedSizeBlob(sizeof(FixedSizeBlob1));

            const size_t actualBufferSize = DocTableDescriptor::GetBufferSize(c_capacity, schema);

            const size_t sliceBufferSize = c_anyDocTableBufferOffset + actualBufferSize;

            AlignedBuffer buffer(sliceBufferSize, c_docTableByteAlignment);
            void* alignedBuffer = buffer.GetBuffer();

            // Fill the allocated buffer with arbitrary byte value.
            memset(alignedBuffer, 10, sliceBufferSize);
            for (unsigned i = 0; i < sliceBufferSize; ++i)
            {
                EXPECT_EQ(*(reinterpret_cast<char*>(alignedBuffer) + i), 10);
            }

            DocTableDescriptor docTable(c_capacity, schema, c_anyDocTableBufferOffset);
            for (unsigned i = 0; i < sliceBufferSize; ++i)
            {
                EXPECT_EQ(*(reinterpret_cast<char*>(alignedBuffer) + i), 10);
            }

            // Range [c_anyDocTableBufferOffset, c_anyDocTableBufferOffset + expectedBufferSize)
            // should be zero initialized by the DocTable.
            docTable.Initialize(alignedBuffer);

            for (unsigned i = 0; i < sliceBufferSize; ++i)
            {
                if (i >= c_anyDocTableBufferOffset && i < c_anyDocTableBufferOffset + sliceBufferSize)
                {
                    EXPECT_EQ(*(reinterpret_cast<char*>(alignedBuffer) + i), 0);
                }
                else
                {
                    EXPECT_EQ(*(reinterpret_cast<char*>(alignedBuffer) + i), 10);
                }
            }

            const std::vector<unsigned> blobSizes = { 100, 200 };

            for (DocIndex i = 0; i < c_capacity; ++i)
            {
                const uint8_t blob0Value = (i + 1) % 0xFF;
                TestAllocateBlob(docTable, alignedBuffer, i, variableBlob0, blobSizes[0], blob0Value);

                const uint8_t blob1Value = (i + 2) % 0xFF;
                TestAllocateBlob(docTable, alignedBuffer, i, variableBlob1, blobSizes[1], blob1Value);

                const DocId docId = static_cast<DocId>(i) + 10;
                docTable.SetDocId(alignedBuffer, i, docId);
                EXPECT_EQ(docTable.GetDocId(alignedBuffer, i), docId);
            }


            for (DocIndex i = 0; i < c_capacity; ++i)
            {
                // Expected values in the blobs.
                const uint8_t blob0Value = (i + 1) % 0xFF;
                const uint8_t blob1Value = (i + 2) % 0xFF;

                void* blob0 = docTable.GetVariableSizeBlob(alignedBuffer, i, variableBlob0);
                void* blob1 = docTable.GetVariableSizeBlob(alignedBuffer, i, variableBlob1);

                uint8_t* ptr = reinterpret_cast<uint8_t*>(blob0);
                for (size_t j = 0; j < blobSizes[0]; ++j)
                {
                    EXPECT_EQ(*ptr, blob0Value);
                    ptr++;
                }

                ptr = reinterpret_cast<uint8_t*>(blob1);
                for (size_t j = 0; j < blobSizes[1]; ++j)
                {
                    EXPECT_EQ(*ptr, blob1Value);
                    ptr++;
                }


                FixedSizeBlob0 testBlob0;
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4267)
#endif
                testBlob0.m_field1 = i + 1;
                testBlob0.m_field2 = static_cast<float>(i + 2);
#ifdef _MSC_VER
#pragma warning(pop)
#endif

                {
                    // Populate the fixed size blob 0.
                    void* fixedSizeBlobData0 = docTable.GetFixedSizeBlob(alignedBuffer, i, fixedSizeBlob0);

                    FixedSizeBlob0& blobData0 = *static_cast<FixedSizeBlob0*>(fixedSizeBlobData0);
                    blobData0 = testBlob0;
                }

                {
                    // Test that the fixed size blob 0 now contains our data.
                    void* fixedSizeBlobData0 = docTable.GetFixedSizeBlob(alignedBuffer, i, fixedSizeBlob0);

                    FixedSizeBlob0& blobData0 = *static_cast<FixedSizeBlob0*>(fixedSizeBlobData0);
                    EXPECT_EQ(blobData0, testBlob0);
                }

                // Test fixed size blob 1.
                {
                    // Populate the fixed size blob 0.
                    void* fixedSizeBlobData1 = docTable.GetFixedSizeBlob(alignedBuffer, i, fixedSizeBlob1);

                    FixedSizeBlob1& blobData1 = *static_cast<FixedSizeBlob1*>(fixedSizeBlobData1);
                    blobData1.m_field1 = static_cast<double>(i + 1);
                    blobData1.m_field2 = static_cast<uint64_t>(i + 2);
                }

                {
                    // Test that the fixed size blob 1 now contains our data.
                    void* fixedSizeBlobData1 = docTable.GetFixedSizeBlob(alignedBuffer, i, fixedSizeBlob1);

                    FixedSizeBlob1& blobData1 = *static_cast<FixedSizeBlob1*>(fixedSizeBlobData1);
                    EXPECT_EQ(blobData1.m_field1, static_cast<double>(i + 1));
                    EXPECT_EQ(blobData1.m_field2, static_cast<uint64_t>(i + 2));
                }
            }

            docTable.Cleanup(alignedBuffer);
            for (DocIndex i = 0; i < c_capacity; ++i)
            {
                void* blob0 = docTable.GetVariableSizeBlob(alignedBuffer, i, variableBlob0);
                EXPECT_EQ(blob0, nullptr);

                void* blob1 = docTable.GetVariableSizeBlob(alignedBuffer, i, variableBlob1);
                EXPECT_EQ(blob1, nullptr);
            }
        }


        void TestBufferSize(DocIndex capacity,
                            IDocumentDataSchema const & schema,
                            unsigned expectedBufferSize)
        {
            const size_t actualBufferSize = DocTableDescriptor::GetBufferSize(capacity, schema);
            const size_t expectedBufferSizeRounded = RoundUp(expectedBufferSize, c_docTableByteAlignment);
            EXPECT_EQ(actualBufferSize, expectedBufferSizeRounded);
        }


        TEST(BufferSizeTest, Trivial)
        {
            static const DocIndex c_capacity = 1000;

            DocumentDataSchema schema;

            // 0 bytes for variable size blobs.
            // sizeof(DocId) = 8 bytes for fixed size storage.
            // 8 is already a power of two.
            // 8 * 1000 (capacity) = 8000.
            TestBufferSize(c_capacity, schema, 8000);

            schema.RegisterVariableSizeBlob();
            // Added 12 bytes for pointer to a variable size blob.
            // 8 + 12 = 20, rounded up to power of 2 is 32.
            TestBufferSize(c_capacity, schema, 32000);

            // Add 5 bytes to a fixed size storage.
            /* const FixedSizeBlobId fixedSizeBlob0 = */schema.RegisterFixedSizeBlob(5);
            // 8 + 12 + 5 = 25, rounded up to power of 2 is 32.
            TestBufferSize(c_capacity, schema, 32000);

            /* const FixedSizeBlobId fixedSizeBlob1 = */schema.RegisterFixedSizeBlob(7);
            // 8 + 12 + 5 + 7 = 32.
            TestBufferSize(c_capacity, schema, 32000);

            /* const FixedSizeBlobId fixedSizeBlob2 = */schema.RegisterFixedSizeBlob(1);
            // 8 + 12 + 5 + 7 + 1 = 33, rounded to the next power of two is 64.
            TestBufferSize(c_capacity, schema, 64000);

            schema.RegisterVariableSizeBlob();
            // 8 + 24 + 5 + 7 + 1 + 12 = 45, rounded up to power of 2 is 64.
            TestBufferSize(c_capacity, schema, 64000);
        }
    }
}
