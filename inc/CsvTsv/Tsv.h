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


namespace CsvTsv
{
    //*************************************************************************
    //
    // TsvTableParser
    //
    // TsvTableParser is an ITableParser for use with TableReader. The class
    // handles the TSV file format. There is no real specification for TSV.
    // this class makes the following assumptions:
    //      Fields may contain any character but '\n', '\r', '\t', '\0'.
    //      Fields are delimited by '\t'.
    //      Leading/trailing whitespace is considered part of the field's value.
    //      Rows are delimited by '\n', \n\r', or '\r\n'.
    //      It is illegal to have two consecutive row delimiters.
    //
    //*************************************************************************
    class TsvTableParser : public DelimitedTableParser
    {
    public:
        TsvTableParser(std::istream& input);
        ~TsvTableParser();

        //
        // ITableParser methods via DelimitedTableParser
        //

        void ReadPrologue(std::vector<std::string>& columns);

        bool ReadComment(std::string& comment);

        // Attempt to parse the next field as a string, int, double or bool.
        // If successful, the value of the field will be stored in value and the
        // functionwill return true. On return the current position will be
        // immediately after the field just returned.
        bool TryReadField(std::string& value);
    };

    //*************************************************************************
    //
    // TsvTableFormatter
    //
    // For use with TableWriter.
    // Formats tabular data in the TSV format.
    //
    //*************************************************************************
    class TsvTableFormatter : public DelimitedTableFormatter
    {
    public:
        TsvTableFormatter(std::ostream& out);

        //
        // ITableFormatter methods via DelimiatedTableFormatter
        //
        void WritePrologue(const std::vector<const char*>& columns);

    private:
        void WriteEscapedValue(const char* value);
    };
}
