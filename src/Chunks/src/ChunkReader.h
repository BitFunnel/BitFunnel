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

#include "BitFunnel/NonCopyable.h"  // Base class.
#include "BitFunnel/Term.h"         // Term::StreamId return value.


namespace BitFunnel
{
    class IChunkProcessor;

    //*************************************************************************
    //
    // ChunkReader
    //
    // Parses a buffer of documents encoded in the BitFunnel chunk format,
    // generating callbacks to an IChunkProcessor.
    //
    //*************************************************************************
    class ChunkReader : public NonCopyable
    {
    // DESIGN NOTE: Need to add arena allocators.
    public:
        ChunkReader(char const * start,
                    char const * end,
                    IChunkProcessor& processor);

    private:
        void ProcessDocument();
        void ProcessStream();
        char const * GetToken();

        DocId GetDocId();
        Term::StreamId GetStreamId();

        uint64_t GetHexValue(uint64_t digitCount);
        void Consume(char c);
        char GetChar();
        char PeekChar();

        // Construtor parameters.
        IChunkProcessor& m_processor;

        // Next character to be processed.
        char const * m_next;

        // Pointer to character beyond the end of m_input.
        char const * m_end;
    };
}
