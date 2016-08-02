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


#include <string.h>
#include <memory>
#include <ostream>


#include "CsvTsv/DelimitedTable.h"


namespace CsvTsv
{
    // TODO: Add line number support by routing all Peek(), EOF(), GetChars(),
    // PutChars() through functions in DelimitedTableParser and
    // DelimitedTableFormatter, and then restrict access to the input and output
    // streams. Copy error reporting mechanism from JSON parser/formatter.
    // TODO: Consider converting all signed char to int. Check related bug that Daniel
    // found and fixed in another file.

    //*************************************************************************
    //
    // DelimitedTableParser
    //
    //*************************************************************************
    DelimitedTableParser::DelimitedTableParser(std::istream& input,
                                               char fieldDelimiter)
        : m_input(&input),
          m_fieldDelimiter(fieldDelimiter),
          m_currentPositionInLine(0),
          m_currentField(0),
          m_currentRow(0)
    {
    }


    DelimitedTableParser::~DelimitedTableParser()
    {
    }


    // Returns true if the current position is immediately before the end of a
    // line (or the end of file).
    bool DelimitedTableParser::AtEOL() const
    {
        // WARNING: Must check for m_input->eof() after peek() to ensure
        // correct eof detection.
        return m_input->peek() == '\n' || m_input->peek() == '\r' || m_input->peek() == -1 || AtEOF();
    }


    // Returns true if the current position is at the end of the stream.
    bool DelimitedTableParser::AtEOF() const
    {
        return m_input->peek() == -1 || m_input->eof();
    }


    void DelimitedTableParser::ReadEpilogue()
    {
        // Default implementation has no epilogue.
    }


    // Moves the current position past the next field on the line if there are
    // fields remaining. Does not advance to the next line. Returns true if the
    // current position after the advance attempt is immediately before the end
    // of a line.
    bool DelimitedTableParser::TrySkipField()
    {
        std::string s;
        return TryReadField(s);
    }


    // Attempts to move the current position to the beginning of the next line,
    // skipping over any remaining fields on the current line. Returns true if
    // the current position after the advance attempt is at the end of the
    // stream.
    bool DelimitedTableParser::AdvanceToNextRow()
    {
        bool success = true;

        if (m_input->peek() == '\n')
        {
            GetChar();
            if (m_input->peek() == '\r')
            {
                GetChar();
            }
            ++m_currentRow;
            m_currentPositionInLine = 0;
            m_currentField = 0;
        }
        else if (m_input->peek() == '\r')
        {
            GetChar();
            if (m_input->peek() == '\n')
            {
                GetChar();
            }
            ++m_currentRow;
            m_currentPositionInLine = 0;
            m_currentField = 0;
        }
        else
        {
            success = false;
        }

        return success;
    }


    unsigned DelimitedTableParser::GetPositionInLine() const
    {
        return m_currentPositionInLine;
    }


    unsigned DelimitedTableParser::GetFieldNumber() const
    {
        return m_currentField;
    }


    unsigned DelimitedTableParser::GetRowNumber() const
    {
        return m_currentRow;
    }


    char DelimitedTableParser::GetChar()
    {
        // TODO: eliminate this cast by changing API to int.
        char c = static_cast<char>(m_input->get());

        // TODO: make sure currentPositionInLine can't get incremented on
        // attempted get at EOF.
        if (m_input->good())
        {
            ++m_currentPositionInLine;
        }
        return c;
    }


    void DelimitedTableParser::ReadLine(std::string& line)
    {
        line.clear();
        while (!AtEOL())
        {
            line.push_back(GetChar());
        }
        AdvanceToNextRow();
    }


    void DelimitedTableParser::ReadColumnNames(const char* current,
                                               std::vector<std::string>& columns)
    {
        while (*current != 0)
        {
            // Add column name to the vector.
            columns.push_back(std::string());
            while (*current != 0 && * current != m_fieldDelimiter)
            {
                columns.back().push_back(*current);
                current++;
            }

            // If there is another field, skip over the comma.
            if (*current == m_fieldDelimiter)
            {
                current++;
            }
        }
    }


    //*************************************************************************
    //
    // DelimitedTableFormatter
    //
    //*************************************************************************
    DelimitedTableFormatter::DelimitedTableFormatter(std::ostream& out,
                                                     char fieldDelimiter,
                                                     char quoteChar)
        : m_fieldDelimiter(fieldDelimiter),
          m_quoteChar(quoteChar),
          m_hexMode(false),
          m_out(&out),
          m_currentField(0),
          m_currentRow(0)
    {
    }


    DelimitedTableFormatter::~DelimitedTableFormatter()
    {
    }


    void DelimitedTableFormatter::WriteColumns(const std::vector<const char*>& columns)
    {
        for (unsigned i = 0; i < columns.size(); ++i)
        {
            WriteField(columns[i]);
        }
        WriteRowEnd();
    }


    void DelimitedTableFormatter::SetHexMode(bool hexMode)
    {
        m_hexMode = hexMode;
    }


    void DelimitedTableFormatter::WriteField(const char* value)
    {
        WriteFieldDelimiter();
        if (strchr(value, m_fieldDelimiter) != NULL
            || (m_quoteChar != 0 && strchr(value, m_quoteChar) != NULL)     // m_quoteChar == 0 when there is no quote character.
            || strchr(value, '\n') != NULL
            || strchr(value, '\r') != NULL)
        {
            WriteEscapedValue(value);
        }
        else
        {
            *m_out << value;
        }
    }


    void DelimitedTableFormatter::WriteField(const std::string& value)
    {
        WriteField(value.c_str());
    }


    void DelimitedTableFormatter::WriteField(int value)
    {
        WriteFieldDelimiter();
        if (m_hexMode)
        {
            *m_out << std::hex << value << std::dec;
        }
        else
        {
            *m_out << value;
        }
    }


    void DelimitedTableFormatter::WriteField(unsigned int value)
    {
        WriteFieldDelimiter();
        if (m_hexMode)
        {
            *m_out << std::hex << value << std::dec;
        }
        else
        {
            *m_out << value;
        }
    }


    void DelimitedTableFormatter::WriteField(unsigned long int value)
    {
        WriteFieldDelimiter();
        if (m_hexMode)
        {
            *m_out << std::hex << value << std::dec;
        }
        else
        {
            *m_out << value;
        }
    }


    void DelimitedTableFormatter::WriteField(long long int value)
    {
        WriteFieldDelimiter();
        if (m_hexMode)
        {
            *m_out << std::hex << value << std::dec;
        }
        else
        {
            *m_out << value;
        }
    }


    void DelimitedTableFormatter::WriteField(unsigned long long int value)
    {
        WriteFieldDelimiter();
        if (m_hexMode)
        {
            *m_out << std::hex << value << std::dec;
        }
        else
        {
            *m_out << value;
        }
    }


    void DelimitedTableFormatter::WriteField(double value)
    {
        WriteFieldDelimiter();
        *m_out << value;
    }


    void DelimitedTableFormatter::WriteField(bool value)
    {
        WriteFieldDelimiter();
        *m_out << (value ? "TRUE" : "FALSE");
    }


    void DelimitedTableFormatter::WriteEmptyField()
    {
        WriteFieldDelimiter();
    }


    void DelimitedTableFormatter::WriteRowEnd()
    {
        // Using "\n" here instead of std::endl to reinforce the point that
        // the file format specification explicitly states "\n".
        // TODO: REVIEW: But what if this is a text stream? Won't the stream
        // replace "\n" with "\n\r"?
        *m_out << "\n";
        m_currentField = 0;
        m_currentRow++;
    }


    void DelimitedTableFormatter::WriteEpilogue()
    {
        // Default implementation has no epilogue.
    }


    void DelimitedTableFormatter::WriteFieldDelimiter()
    {
        if (m_currentField > 0)
        {
            // If this is not the first field on a line, emit a field delimiter.
            *m_out << m_fieldDelimiter;
        }
        ++m_currentField;
    }
}
