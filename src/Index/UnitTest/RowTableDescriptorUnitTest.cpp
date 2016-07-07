#include "stdafx.h"

#include <algorithm>
#include <set>
#include <vector>

#include "BitFunnel/NonCopyable.h"
#include "BitFunnel/Row.h"
#include "BitFunnel/TermInfo.h"
#include "EmptyTermTable.h"
#include "Random.h"
#include "Rounding.h"
#include "RowTableDescriptor.h"
#include "SuiteCpp/UnitTest.h"


namespace BitFunnel
{
    namespace RowTableDescriptorUnitTest
    {
        class Bit
        {
        public:
            Bit(RowIndex row, DocIndex column);

            RowIndex GetRow() const;
            DocIndex GetColumn() const;

            bool operator<(Bit const & other) const;

        private:
            RowIndex m_row;
            DocIndex m_column;
        };


        Bit::Bit(RowIndex row, DocIndex column)
            : m_row(row),
              m_column(column)
        {
        }


        RowIndex Bit::GetRow() const
        {
            return m_row;
        }


        DocIndex Bit::GetColumn() const
        {
            return m_column;
        }


        bool Bit::operator<(Bit const & other) const
        {
            if (m_row == other.m_row)
            {
                return m_column < other.m_column;
            }
            else
            {
                return m_row < other.m_row;
            }
        }


        // RAII helper class which allocates, initializes the buffer required for the RowTable
        // and deallocates when the object is out of scope.
        class RowTableHolder : NonCopyable
        {
        public:
            RowTableHolder(DocIndex capacity, 
                           ITermTable const & termTable, 
                           Rank rank,
                           ptrdiff_t rowTableOffset);

            ~RowTableHolder();

            RowTableDescriptor const & GetRowTable() const;
            RowTableDescriptor& GetRowTable();

            void* GetBuffer() const;

        private:
            ptrdiff_t m_rowTableOffsetInBuffer;
            size_t m_bufferSize;
            void* m_buffer;
            RowTableDescriptor m_rowTable;

        };


        RowTableHolder::RowTableHolder(DocIndex capacity, 
                                       ITermTable const & termTable, 
                                       Rank rank, 
                                       ptrdiff_t rowTableOffset)
            : m_rowTableOffsetInBuffer(static_cast<unsigned>(RoundUp(rowTableOffset, c_rowTableByteAlignment))),
              m_bufferSize(m_rowTableOffsetInBuffer + RowTableDescriptor::GetBufferSize(capacity, 
                                                                                        termTable.GetTotalRowCount(DDRTier, rank), 
                                                                                        rank)),
              m_buffer(new char[m_bufferSize]),
              m_rowTable(capacity, termTable.GetTotalRowCount(DDRTier, rank), rank, m_rowTableOffsetInBuffer)
        {
            char const * buffer = reinterpret_cast<char*>(m_buffer);

            // Initially set the buffer to be garbage data.
            memset(m_buffer, 1, m_bufferSize);

            for (unsigned i = 0; i < m_bufferSize; ++i)
            {
                TestEqual(*(buffer + i), 1);
            }
            
            // Initialize this RowTable's portion in the buffer and verify.
            m_rowTable.Initialize(m_buffer, termTable);

            const size_t rowTableBufferSize = RowTableDescriptor::GetBufferSize(capacity, 
                                                                                termTable.GetTotalRowCount(DDRTier, rank), 
                                                                                rank);

            if (rank == 0)
            {
                // At rank 0 we have a special row which is initialized with all bits set to 1 (match-all row).
                // The range [m_rowTableOffsetInBuffer, m_rowTableOffsetInBuffer + rowTableBufferSize)
                // is expected to be zeros except for the range which is reserved for match-all row.
                TermInfo termInfo(ITermTable::GetMatchAllTerm(), termTable);
                TestAssert(termInfo.MoveNext());
                const RowId matchAllRowId = termInfo.Current();
                TestAssert(!termInfo.MoveNext());
                const ptrdiff_t matchAllStart = m_rowTable.GetRowOffset(matchAllRowId.GetIndex());
                const ptrdiff_t matchAllEnd = matchAllStart + capacity / 8;

                // The range [m_rowTableOffsetInBuffer, m_rowTableOffsetInBuffer + rowTableBufferSize)
                // is expected to be zeros.
                for (unsigned i = 0; i < m_bufferSize; ++i)
                {
                    if (i >= m_rowTableOffsetInBuffer && i < m_rowTableOffsetInBuffer + rowTableBufferSize)
                    {
                        if (i >= matchAllStart && i < matchAllEnd)
                        {
                            // -1 = 0xFF.
                            TestEqual(*(buffer + i), -1);
                        }
                        else
                        {
                            TestEqual(*(buffer + i), 0);
                        }
                    }
                    else
                    {
                        TestEqual(*(buffer + i), 1);
                    }
                }
            }
            else
            {
                // The range [m_rowTableOffsetInBuffer, m_rowTableOffsetInBuffer + rowTableBufferSize)
                // is expected to be zeros.
                for (unsigned i = 0; i < m_bufferSize; ++i)
                {
                    if (i >= m_rowTableOffsetInBuffer && i < m_rowTableOffsetInBuffer + rowTableBufferSize)
                    {
                        TestEqual(*(buffer + i), 0);
                    }
                    else
                    {
                        TestEqual(*(buffer + i), 1);
                    }
                }
            }
        }


        RowTableHolder::~RowTableHolder()
        {
            delete[] m_buffer;
        }


        RowTableDescriptor const & RowTableHolder::GetRowTable() const
        {
            return m_rowTable;
        }


        RowTableDescriptor& RowTableHolder::GetRowTable()
        {
            return m_rowTable;
        }


        void* RowTableHolder::GetBuffer() const
        {
            return m_buffer;
        }


        // Helper function which verifies if all bits are set as expected.
        // The expected parameter will be modified to remove the bits already
        // verified.
        void Validate(DocIndex capacity,
                      RowIndex rowCount,
                      RowTableHolder const & rowTableHolder,
                      std::vector<Bit>& expected)
        {
            // Arrange expected bits first by ascending rows and then by
            // ascending columns.
            std::sort(expected.begin(), expected.end());

            RowTableDescriptor const & rowTable = rowTableHolder.GetRowTable();
            void* buffer = rowTableHolder.GetBuffer();

            // Walk row table bits in same order as expected bits.
            unsigned current = 0;
            for (RowIndex row = 0; row < rowCount; ++row)
            {
                for (DocIndex column = 0; column < capacity; ++column)
                {
                    // If a bit is set in the RowTable, verify that it was
                    // expected.
                    if (rowTable.GetBit(buffer, row, column) != 0)
                    {
                        TestEqual(expected[current].GetRow(), row);
                        TestEqual(expected[current].GetColumn(), column);

                        // Advance to the next expected bit that has a
                        // different (row, column). Need to use a while loop
                        // here to deal with duplicate bits created by the
                        // random number generator.
                        while (current < expected.size()
                               && expected[current].GetRow() == row
                               && expected[current].GetColumn() == column)
                        {
                            ++current;
                        }
                    }
                }
            }
            TestEqual(current, expected.size());
        }


        static void VerifySetAndClear(RowTableHolder& rowTableHolder, RowIndex row, DocIndex column)
        {
            RowTableDescriptor& rowTable = rowTableHolder.GetRowTable();
            void* const buffer = rowTableHolder.GetBuffer();

            // Ensure the bit is clear before starting.
            TestAssert(rowTable.GetBit(buffer, row, column) == 0);

            // Set the bit and verify.
            rowTable.SetBit(buffer, row, column);
            TestAssert(rowTable.GetBit(buffer, row, column) == 1);

            // Clear the bit and verify.
            rowTable.ClearBit(buffer, row, column);
            TestAssert(rowTable.GetBit(buffer, row, column) == 0);
        }


        unsigned GetRowTableSize(DocIndex capacity, RowIndex rowCount, Rank rank)
        {
            return Row::DocumentsInRank0Row(capacity) * rowCount / (2 << (3 + rank));
        }


        TestCase(BitVectorContents)
        {
            const RowIndex c_rowCount = 10;
            const DocIndex c_columnCount = Row::DocumentsInRank0Row(100);

            //
            // Rank 0
            //
            {
                const std::vector<RowIndex> rowCounts = { c_rowCount, 0, 0, 0, 0, 0, 0 };
                EmptyTermTable termTable(rowCounts);
                RowTableHolder holder(c_columnCount, termTable, 0, 123);

                // At rank 0 there are certain system rows, skip them for the test.
                const RowIndex c_publicRowCount = c_rowCount - c_systemRowCount;
                for (DocIndex column = 0; column < c_columnCount; ++column)
                {
                    VerifySetAndClear(holder, (column % c_publicRowCount) + c_systemRowCount, column);
                }
            }

            //
            // Rank 1
            //
            {
                const std::vector<RowIndex> rowCounts = { c_systemRowCount, c_rowCount, 0, 0, 0, 0, 0 };
                EmptyTermTable termTable(rowCounts);
                RowTableHolder holder(c_columnCount, termTable, 1, 456);

                // First two quad words at rank 0 map to first quad word at rank 1.
                VerifySetAndClear(holder, 0, 0);
                VerifySetAndClear(holder, 0, 64);

                // Next two quad words at rank 0 map to second quad word at rank 1.
                VerifySetAndClear(holder, 0, 128);
                VerifySetAndClear(holder, 0, 192);
            }

            //
            // Rank 2
            //
            {
                const std::vector<RowIndex> rowCounts = { c_systemRowCount, 0, c_rowCount, 0, 0, 0, 0 };
                EmptyTermTable termTable(rowCounts);
                RowTableHolder holder(c_columnCount, termTable, 2, 789);

                // First four quad words at rank 0 map to first quad word at rank 2.
                VerifySetAndClear(holder, 0, 0);
                VerifySetAndClear(holder, 0, 64);
                VerifySetAndClear(holder, 0, 128);
                VerifySetAndClear(holder, 0, 192);

                // Next four quad words at rank 0 map to second quad word at rank 2.
                VerifySetAndClear(holder, 0, 256);
                VerifySetAndClear(holder, 0, 320);
                VerifySetAndClear(holder, 0, 384);
                VerifySetAndClear(holder, 0, 448);
            }
        }


        void TestBufferSize(DocIndex capacity, RowIndex rowCount, Rank rank, size_t expectedBufferSize)
        {
            const size_t actualBufferSize = RowTableDescriptor::GetBufferSize(capacity, rowCount, rank);
            TestEqual(actualBufferSize, expectedBufferSize);
        }


        TestCase(BufferSizeTest)
        {
            static const DocIndex c_capacityQuanta = Row::DocumentsInRank0Row(1);

            TestBufferSize(c_capacityQuanta * 1, 100, 0, 51200);
            TestBufferSize(c_capacityQuanta * 2, 50, 0, 51200);
            TestBufferSize(c_capacityQuanta * 1, 10, 0, 5120);

            TestBufferSize(c_capacityQuanta * 1, 10, 3, 640);
            TestBufferSize(c_capacityQuanta * 1, 10, 6, 80);
            TestBufferSize(c_capacityQuanta * 10, 1, 6, 80);
            TestBufferSize(c_capacityQuanta * 10, 10, 6, 800);
        }


        TestCase(RowOffsetTest)
        {
            {
                // Rank 0.
                const DocIndex c_sliceCapacity = Row::DocumentsInRank0Row(4096) * 3;
                const RowIndex c_rowCount = 100;
                const Rank c_rank = 0;
                const ptrdiff_t c_bufferOffset = 123 * c_rowTableByteAlignment;
                RowTableDescriptor rowTable(c_sliceCapacity, c_rowCount, c_rank, c_bufferOffset);

                // 1536 bytes per row.
                TestEqual(rowTable.GetRowOffset(0), c_bufferOffset);
                TestEqual(rowTable.GetRowOffset(10), c_bufferOffset + 1536 * 10);
                TestEqual(rowTable.GetRowOffset(99), c_bufferOffset + 1536 * 99);
            }

            {
                // Rank 3.
                const DocIndex c_sliceCapacity = Row::DocumentsInRank0Row(4096) * 2;
                const RowIndex c_rowCount = 200;
                const Rank c_rank = 3;
                const ptrdiff_t c_bufferOffset = 31 * c_rowTableByteAlignment;
                RowTableDescriptor rowTable(c_sliceCapacity, c_rowCount, c_rank, c_bufferOffset);

                // 128 bytes per row.
                TestEqual(rowTable.GetRowOffset(0), c_bufferOffset);
                TestEqual(rowTable.GetRowOffset(10), c_bufferOffset + 128 * 10);
                TestEqual(rowTable.GetRowOffset(49), c_bufferOffset + 128 * 49);
            }
        }
    }
}
