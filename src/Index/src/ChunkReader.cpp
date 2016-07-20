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

#include <sstream>

#include "BitFunnel/Exceptions.h"
#include "ChunkReader.h"


namespace BitFunnel
{
    static const uint64_t c_docIdDigitCount = 16;
    static const uint64_t c_streamIdDigitCount = 2;


    ChunkReader::ChunkReader(std::vector<char> const & input, IEvents& processor)
        : m_processor(processor),
          m_next(&input[0]),
          m_end(&input[0] + input.size())
    {
        if (m_next == m_end) {
            throw FatalError("Attempt to read empty chunk.");
        }

        m_processor.OnFileEnter();
        while (PeekChar() != 0) {
            ProcessDocument();
        }

        Consume(0);
        m_processor.OnFileExit();
    }


    void ChunkReader::ProcessDocument()
    {
        uint64_t id = GetDocId();
        m_processor.OnDocumentEnter(id);
        while (PeekChar() != 0) {
            ProcessStream();
        }

        Consume(0);

        m_processor.OnDocumentExit();
    }


    void ChunkReader::ProcessStream()
    {
        Term::StreamId id = GetStreamId();
        m_processor.OnStreamEnter(id);
        while (PeekChar() != 0) {
            m_processor.OnTerm(GetToken());
        }

        Consume(0);

        m_processor.OnStreamExit();
    }


    char const * ChunkReader::GetToken()
    {
        char const * begin = m_next;

        while (PeekChar() != 0) {
            GetChar();
        }

        Consume(0);

        return begin;
    }

    DocId ChunkReader::GetDocId()
    {
        static_assert(sizeof(DocId) * 2 == c_docIdDigitCount,
                      "DocId type is not equivalent to two hex digits.");
        return static_cast<DocId>(GetHexValue(c_docIdDigitCount));
    }


    Term::StreamId ChunkReader::GetStreamId()
    {
        static_assert(sizeof(Term::StreamId) * 2 == c_streamIdDigitCount,
                      "Term::StreamId type is not equivalent to two hex digits.");
        return static_cast<Term::StreamId>(GetHexValue(c_streamIdDigitCount));
    }


    uint64_t ChunkReader::GetHexValue(uint64_t digitCount)
    {
        uint64_t value = 0;
        for (unsigned i = 0; i < digitCount; ++i)
        {
            value <<= 4;
            char c = PeekChar();
            if (c >= '0' && c <= '9')
            {
                value |= (c - '0');
            }
            else if (c >= 'a' && c <= 'f')
            {
                value |= (c - 'a' + 10);
            }
            else
            {
                throw FatalError("Expected hexidecimal digit from {0123456789abcdef}.");
            }
            GetChar();
        }
        Consume('\0');

        return value;
    }


    char ChunkReader::PeekChar()
    {
        if (m_next == m_end) {
            throw FatalError("Attempt to read beyond end of buffer.");
        }
        else
        {
            return *m_next;
        }
    }


    char ChunkReader::GetChar()
    {
        if (m_next == m_end) {
            throw FatalError("Attempt to read beyond end of buffer.");
        }
        else
        {
            return *m_next++;
        }
    }

    void ChunkReader::Consume(char c)
    {
        if (PeekChar() != c) {
            std::stringstream msg;
            msg << "Expected character " << c << ". ";
            msg << "Found character " << PeekChar() << ".";
            throw FatalError(msg.str());
        }

        GetChar();
    }
}
