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


#include "BitFunnel/Row.h"

#include <inttypes.h>
#include <stddef.h>

#include "BitFunnel/Index/DocumentHandle.h"  // For DocIndex.

namespace BitFunnel
{
    // Helper method.
    template <typename T>
    static T RoundUp(T value, T quanta)
    {
        return ((value + quanta - 1) / quanta) * quanta;
    }


    //*************************************************************************
    //
    // Row
    //
    //*************************************************************************
    Row::Row(uint64_t const * data)
        : m_data(data)
    {
    }


    uint64_t const * Row::GetData() const
    {
        return m_data;
    }


    unsigned Row::GetAlignment()
    {
        return c_byteAlignment;
    }


    uint64_t* Row::Align(void* address)
    {
        return reinterpret_cast<uint64_t*>
            ((reinterpret_cast<size_t>(address) +
              static_cast<size_t>(c_byteAlignment) - 1) &
             ~(static_cast<size_t>(c_byteAlignment) - 1));
    }


    size_t Row::BytesInRow(DocIndex documentCount, Rank rowRank)
    {
        return DocumentsInRank0Row(documentCount) >> (3ull + rowRank);
    }


    DocIndex Row::DocumentsInRank0Row(DocIndex documentCount)
    {
        // DESIGN NOTE: Because the matching engine does quadword loads, it
        // is necessary that c_byteAlignment be at least 8 so that
        // DocumentsInRank0Row() correctly pads the row length.
        static const unsigned c_documentsPerByte = 8;
        unsigned rowQuanta =
            (c_byteAlignment * c_documentsPerByte) << c_maxRankValue;

        return RoundUp<size_t>(documentCount, rowQuanta);
    }
}
