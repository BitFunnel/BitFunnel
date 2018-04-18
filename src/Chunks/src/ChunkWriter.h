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

#include <stdint.h>
#include <vector>

#include "BitFunnel/Chunks/IChunkProcessor.h"


namespace BitFunnel
{

    //*************************************************************************
    //
    // ChunkWriter
    //
    // Outputs a buffer of documents encoded in the BitFunnel chunk format
    // to an output stream
    //
    //*************************************************************************
    class ChunkWriter : public IChunkWriter
    {
    public:
        ChunkWriter(char const * start,
            char const * end);

        // Writes the bytes in range [m_start, m_end) to the specified
        // stream. ChunkReader uses this method to write the range of bytes
        // corresponding to a single document.
        void Write(std::ostream & output) override;

        // Writes a single '\0' to the specified stream. ChunkReader uses
        // this method to write the closing `\0` after a sequence of
        // documents.
        void Complete(std::ostream & output) override;

    private:
        char const * m_start;
        char const * m_end;
    };
}
