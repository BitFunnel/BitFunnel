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

#include "DelimitedTable.h"
#include "ParseError.h"
#include "Table.h"


namespace CsvTsv
{
    //*************************************************************************
    //
    // CsvTableParser
    //
    // CsvTableParser is an ITableParser for use with TableReader. The class
    // handles the CSV file format as specified by 
    //      http://tools.ietf.org/html/rfc4180.
    //
    // CsvTableParser also supports Bing header comments of the form
    // #Fields:column1,column2,...
    //
    //*************************************************************************
    class CsvTableParser : public DelimitedTableParser
    {
    public:
        // Constructs a CsvTableParser on a specified input stream. Error
        // messages, including line numbers and character positions for
        // parse errors will go to the error stream.
        CsvTableParser(std::istream& input);

        ~CsvTableParser();

        // When HeaderCommentMode is set to true, the header row begins with
        // "#Fields:", followed by the column names. Otherwise, the header row
        // consists solely of column names.
        void SetHeaderCommentMode(bool mode);

        // Reads the prologue portion of the file. This is the portion before
        // the data rows. The prologue contains a header with column names.
        // These names are extracted and added to the columns vector parameter.
        void ReadPrologue(std::vector<std::string>& columns);

        // Attempts to read a comment at the current position. If successful,
        // the entire comment (including special comment delimiters) is stored
        // in the comment parameter and the method returns true and the current
        // position is immediately after the comment. Otherwise the method
        // returns false and the current position is unchanged.
        bool ReadComment(std::string& comment);

        // Attempt to parse the next field as a string, int, double or bool.
        // If successful, the value of the field will be stored in value and
        // the function will return true. On return the current position will
        // be immediately after the field just returned.
        bool TryReadField(std::string& value);

    private:

        // When m_headerCommentMode is true, the CsvTableParser will get its
        // column names from a comment in the prologue of the form
        // #Fields:name1,name2,...
        // When m_headerCommentMode is false, the CsvTableParser will get its
        // column names from the first row.
        bool m_headerCommentMode;
    };


    //*************************************************************************
    //
    // CsvTableFormatter
    //
    // For use with TableWriter.
    // Formats tabular data according to http://tools.ietf.org/html/rfc4180.
    //
    // CsvTableFormatter also supports Bing header comments of the form
    // #Fields:column1,column2,...

    //*************************************************************************
    class CsvTableFormatter : public DelimitedTableFormatter
    {
    public:
        // Constructs a CsvTableFormatter that writes to a specified stream.
        CsvTableFormatter(std::ostream& out);

        // When HeaderCommentMode is set to true, the header row begins with
        // "#Fields:", followed by the column names. Otherwise, the header row
        // consists solely of column names.
        void SetHeaderCommentMode(bool mode);

        // Override of DelimitedTableFormatter::WritePrologue(). When
        // m_headerCommentMode is true, writes a column definition comment
        // beginning with #Fields:.
        void WritePrologue(const std::vector<const char*>& columns);

    private:
        // Escapes value string according to http://tools.ietf.org/html/rfc418
        // and writes to the output stream.
        void WriteEscapedValue(const char* value);

        // When m_headerCommentMode is true, the CsvTableFormatter will write
        // the column name header in the form of a comment:
        // #Fields:name1,name2,...
        // When m_headerCommentMode is false, the CsvTableFormatter will write
        // the column name header as the first row.
        bool m_headerCommentMode;
    };


    // Convenience function to read in a column of values of type T.
    // Does not read in a column header. Does not handle comments.
    template <class T>
    void ReadCsvColumn(std::istream& input, std::vector<T>& values)
    {
        CsvTableParser parser(input);

        // TODO: Better, more specific error message.
        InputColumn<T> column("Input file", "Expected one field per line.");

        while (!parser.AtEOF())
        {
            column.Read(parser);
            values.push_back(column);

            if (!parser.AtEOL())
            {
                ParseErrorBuilder error(parser);
                error << "expected one field per line";
                error.Throw();
            }

            parser.AdvanceToNextRow();
        }
    }


    // Convenience function to write out a column of values of type T.
    // Does not write a column header.
    template <class T>
    void WriteCsvColumn(std::ostream& output, const std::vector<T>& values)
    {
        CsvTableFormatter formatter(output);
        
        for (unsigned i = 0 ; i < values.size(); ++i)
        {
            formatter.WriteField(values[i]);
            formatter.WriteRowEnd();
        }
    }
}
