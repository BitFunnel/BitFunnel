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

#include "BitFunnel/BitFunnelTypes.h"
#include "BitFunnel/Exceptions.h"
#include "BitFunnel/PackedRowIdSequence.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // TermTable::PackedRowIdSequence
    //
    //*************************************************************************
    PackedRowIdSequence::PackedRowIdSequence(RowIndex start,
                                             RowIndex end,
                                             Type type)
      : m_start(static_cast<uint32_t>(start)),
        m_count(static_cast<uint32_t>(end - start)),
        m_type(static_cast<uint32_t>(type))
    {
        if (start > c_maxRowIndexValue ||
            end > c_maxRowIndexValue ||
            start > end ||
            end - start > RowConfiguration::Entry::c_maxRowCount ||
            type > c_maxTypeValue)
        {
            RecoverableError error("PackedRowIdSequence: invalid parameter.");
            throw error;
        }
    }
}
