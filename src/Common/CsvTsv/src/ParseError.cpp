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


#include <ostream>

#include "CsvTsv/ParseError.h"
#include "CsvTsv/Table.h"


namespace CsvTsv
{
    Error::Error(const std::string& message)
        : std::runtime_error(message)
    {
    }


    ParseError::ParseError(const std::string& message, unsigned line, unsigned position)
        : Error(message),
          m_line(line),
          m_position(position)
    {
    }


    unsigned ParseError::GetLine() const
    {
        return m_line;
    }


    unsigned ParseError::GetPosition() const
    {
        return m_position;
    }


    void ParseError::FormatErrorMessage(std::ostream& output) const
    {
        output << "Parse error: " << this->what();
        output << " (at line " << m_line << ", position " << m_position << ")";
        output << std::endl;
    }


    ParseErrorBuilder::ParseErrorBuilder(const IFieldParser& parser)
        : m_line(parser.GetRowNumber()),
          m_position(parser.GetPositionInLine())
    {
    }


    void ParseErrorBuilder::Throw() const
    {
        ParseError error(m_message.str(), m_line, m_position);
        throw error;
    }
}
