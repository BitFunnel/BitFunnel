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


#pragma once

#include <cstddef>                      // size_t embedded.

#include "BitFunnel/BitFunnelTypes.h"   // DocIndex parameter.
#include "BitFunnel/RowId.h"            // RowIndex parameter.


namespace BitFunnel
{
    class ITermTable2;

    //*************************************************************************
    //
    // RowTableDescriptor is a helper class which exposes bit operations over a
    // row buffer which is embedded within a larger slice buffer with regions
    // for other RowTable buffers and the DocTable buffer. RowTableDescriptor
    // knows where in the buffer its RowTable starts and what dimensions it has
    // and is able to perform bit operations over that data.
    // See Slice.h for more info about the layout of the data buffer.
    //
    // All methods except Initialize are thread safe. Initialize method is not
    // thread-safe with respect to calling *Bit methods at the same time.
    //
    //*************************************************************************
    class RowTableDescriptor
    {
    public:
        // Constructs a RowTableDescriptor with given dimensions.
        // rowTableBufferOffset represents the offset where this RowTable's
        // data starts within a larger slice buffer which is passed to other
        // methods.
        RowTableDescriptor(DocIndex capacity,
                           RowIndex rowCount,
                           Rank rank,
                           ptrdiff_t bufferOffset);

        // Copy constructor from another RowTableDescriptor. Required so that
        // RowTableDescriptor can be used in std::vector and that a Slice can
        // create a cached copy of the RowTableDescriptor from Shard.
        RowTableDescriptor(RowTableDescriptor const & other);

        // Zero out row buffer. May not be required if buffers come out of
        // allocator zero initialized. Expected to be called one per
        // sliceBuffer. All rows are initialized with zero in all bits except
        // for the "match-all" row. ITermTable determines where this row is
        // located.
        // Not thread safe with respect to calling *Bit methods at the same
        // time.
        void Initialize(void* sliceBuffer, ITermTable2 const & termTable) const;

        // No cleanup method required.

        // Gets a bit in the given row and column.
        uint64_t GetBit(void* sliceBuffer, RowIndex rowIndex, DocIndex docIndex) const;

        // Sets a bit in the given row and column.
        void SetBit(void* sliceBuffer, RowIndex rowIndex, DocIndex docIndex) const;

        // Clears a bit in the given row and column.
        void ClearBit(void* sliceBuffer, RowIndex rowIndex, DocIndex docIndex) const;

        // Returns the offset of a row with the given index, relative to the
        // start of the sliceBuffer.
        ptrdiff_t GetRowOffset(RowIndex rowIndex) const;

        // Returns true if the given RowTableDescriptor is data-compatible with
        // this instance. Used when loading Slices from the stream.
        bool IsCompatibleWith(RowTableDescriptor const & other) const;

        // Returns the byte size of the buffer required to host a RowTable with
        // given dimensions. This assists the caller in allocating large enough
        // buffer for all RowTables.
        static size_t GetBufferSize(DocIndex capacity,
                                    RowIndex rowCount,
                                    Rank rank);

    private:
        // Declare but don't implement. This is required for a std::vector to
        // use a copy constructor instead of assignment operator.
        RowTableDescriptor& operator=(RowTableDescriptor const & other);

        // Helper method to seek to the data for the row with the given
        // RowIndex.
        uint64_t* GetRowData(void* sliceBuffer, RowIndex rowIndex) const;

        // Returns the QWORD number for the given DocIndex.
        size_t QwordPositionFromDocIndex(DocIndex docIndex) const;

        // Returns the bit number for the given DocIndex.
        size_t BitPositionFromDocIndex(DocIndex docIndex) const;

        // Dimensions of the RowTable.
        const DocIndex m_capacity;
        const RowIndex m_rowCount;
        const Rank m_rank;

        // Offset where this RowTable starts in the slice buffer.
        const ptrdiff_t m_bufferOffset;

        // Cached value of the number of bytes per single row.
        const size_t m_bytesPerRow;
    };
}
