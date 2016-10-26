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


#include <istream>
#include <string>
#include <vector>

#include "Table.h"


namespace CsvTsv
{
    //*************************************************************************
    //
    // DelimitedTableParser
    //
    // For use with the class TableReader. DelimitedTableParser is the base
    // class for parsers of files with a single-character field delimiter,
    // an optional single-character quote for escaping the field delimiter,
    // and multi-character row delimiter.
    //
    //*************************************************************************
    class DelimitedTableParser : public ITableParser
    {
    public:
        DelimitedTableParser(std::istream& input,
                             char fieldDelimiter);

        virtual ~DelimitedTableParser();

        //
        // ITableParser methods
        //

        // Returns true if the current position is immediately before the end
        // of a line (or the end of file).
        bool AtEOL() const;

        // Skips over any comments and then returns true if the current
        // position is at the end of the stream.
        bool AtEOF() const;

        // Reads the portion of the file after the data rows.
        virtual void ReadEpilogue();

        // Moves the current position past the next field on the line if there
        // are fields remaining. Does not advance to the next line. Returns
        // true if the current position after the advance attempt is
        // immediately before the end of a line.
        bool TrySkipField();

        // Attempts to move the current position to the beginning of the next
        // line, skipping over any remaining fields on the current line.
        // Returns true if the current position after the advance attempt is at
        // the end of the stream.
        bool AdvanceToNextRow();

        // Returns the current character position relative to beginning of
        // current line.
        unsigned GetPositionInLine() const;

        // Returns the current field position on the current line.
        unsigned GetFieldNumber() const;

        // Returns the current line number.
        unsigned GetRowNumber() const;

    protected:
        // Remove and return one character from the input stream and advance
        // m_positionInLine.
        // TODO: Decide on error handling strategy. Does m_positionInLine get
        // advanced for EOL scenario?
        char GetChar();

        // Reads in the next line delimited by a LF/CR pair and stores the line
        // text in the line parameter. On return, the current positon will be
        // at the beginning of the next line, or at the end of stream if there
        // are no more lines.
        void ReadLine(std::string& line);

        // Parses a string representation of a delimited sequence of column
        // names and stores the resulting names in the columns vector. Names
        // are defined to be sequences of characters delimited by '\0' or
        // m_fieldDelimiter.
        void ReadColumnNames(const char* current,
                             std::vector<std::string>& columns);

        std::istream* m_input;

    private:
        char m_fieldDelimiter;

        unsigned m_currentPositionInLine;
        unsigned m_currentField;
        unsigned m_currentRow;
    };

    //*************************************************************************
    //
    // DelimitedTableFormatter
    //
    // Abstract base class for ITableFormatters that output data in tabular
    // format with a single-character field delimiter, an optional single-character
    // quote for escaping the field delimiter, and multi-character row delimiter
    // equal to "\n", "\n\r", or "\r\n".
    //
    //*************************************************************************
    class DelimitedTableFormatter : public ITableFormatter
    {
    public:
        // Constructs a DelimitedTableFormatter that writes to a specified
        // output stream. The fieldDelimiter parameter specifies the
        // character that seperates values on a line (e.g. ',' for CSV and
        // '\t' for TSV). The quoteChar parameter specifies the character to be
        // used for quoting (e.g. in CSV, a field can contain a comma if it is
        // quoted with "). A value of 0 for quoteChar supresses quoting
        // functionality in the formattiner.
        DelimitedTableFormatter(std::ostream& out, char fieldDelimiter, char quoteChar);

        virtual ~DelimitedTableFormatter();

        //
        // IFieldFormatter methods via ITableFormatter
        //

        // TODO: REVIEW: Is it possible to have once column hex and another decimal?
        // When in hex mode, fields of type int, unsigned int, long, and
        // unsigned long long will be formatted as hexidecimal. This formatting
        // will not include a base prefix (e.g. "0x").
        void SetHexMode(bool hexMode);

        void WriteField(const char* value);
        void WriteField(const std::string& value);
        void WriteField(int value);
        void WriteField(unsigned int value);
        void WriteField(unsigned long int);
        void WriteField(long long int value);
        void WriteField(unsigned long long int value);
        void WriteField(double value);
        void WriteField(bool value);
        void WriteEmptyField();

        //
        // ITableFormatter methods
        //

        void WriteRowEnd();

        void WriteEpilogue();

    protected:
        // Subclasses may override this method to implement escaping (e.g. CvsTableFormatter)
        // or throw an exception for values that contain the field delimiter character
        // (e.g. TsvTableFormatter).
        virtual void WriteEscapedValue(const char* value) = 0;

        // Writes out a row containing the column names. Used by WritePrologue().
        void WriteColumns(const std::vector<const char*>& columns);

        // Writes the field delimiter if the current line already has at least one field.
        void WriteFieldDelimiter();

        char m_fieldDelimiter;
        char m_quoteChar;

        bool m_hexMode;

        std::ostream* m_out;

    private:
        unsigned m_currentField;
        unsigned m_currentRow;
    };
}
