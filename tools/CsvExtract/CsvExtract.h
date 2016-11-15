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

#include <string>                   // std::string parameter.
#include <vector>                   // std::vector parameter.

#include "BitFunnel/IExecutable.h"  // Base class.


namespace BitFunnel
{
    //*************************************************************************
    //
    // CsvExtract
    //
    // Reads a .csv formatted table from the input stream, extracts the columns
    // passed in columnNames, and then writes a new .csv formatted table to
    // the output stream.
    //
    // Columns will written be in the order they appear in columnNames.
    //
    // Notes:
    //   1. columnNames must specify at least one column.
    //   2. All columns in columnNames must appear in the header of the input.
    //   3. The input must start with a header row.
    //
    //*************************************************************************
    class CsvExtract : public IExecutable
    {
    public:
        //
        // IExecutable methods
        //
        virtual int Main(std::istream& input,
                         std::ostream& output,
                         int argc,
                         char const *argv[]) override;

        //
        // Other methods.
        //

        static void Filter(
            std::istream& input,
            std::ostream& output,
            std::vector<std::string> const & columnNames,
            bool plainTextOutput);

    private:
        static void Usage(std::ostream& output);
    };
}
