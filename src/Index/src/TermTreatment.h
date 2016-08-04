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

#include "BitFunnel/IEnumerator.h"      // Inherits from IEnumerator.

namespace BitFunnel
{
    // Design goals:
    //   Use in query pipeline ==> fast, no memory allocations.

    // Issues:
    //   How is the class configured for density, SNR, available ranks?
    //   Is an instance created as each term is processed, or are a fixed number
    //   of instances associated with classes of terms created at startup?

    class RankDescriptor
    {
    public:
        RankDescriptor(RowIndex count, bool isPrivate);

        RowIndex GetRowCount() const;
        bool IsPrivate() const;

    private:
        RowIndex m_count;
        bool m_isPrivate;
    };


    class TermTreatment : public IEnumerator<RankDescriptor>
    {
    public:

        TermTreatment(Term term, double frequency);

        //
        // IEnumerator methods.
        //
        bool MoveNext();
        void Reset();
        RankDescriptor Current() const;

    private:
        uint64_t m_data;
        size_t m_nextRank;
    };
}
