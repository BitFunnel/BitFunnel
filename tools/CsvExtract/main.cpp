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

#include <iostream>
#include <istream>
#include <ostream>
#include <set>
#include <string>

#include "CmdLineParser/CmdLineParser.h"
#include "CsvTsv/Csv.h"
#include "LoggerInterfaces/Check.h"

void Filter(std::istream& input,
            std::ostream& output,
            std::set<std::string> const & columnNames)
{
    // Read first row.
    // Get numbers for each column.
    // Ensure all columns were found.

    // Extract in order presented on command line.
}

void Usage()
{
    std::cout
        << "CsvExtract <input file> <output file> [<column number>]+" << std::endl;
}

int main(int argc, char** argv)
{
    try
    {
        CHECK_GT(argc, 3)
            << "Expect <input file> <output file> [<column number>]+";

        char const * infile = argv[1];
        char const * outfile = argv[2];
        std::set<std::string> columnNames;

        for (size_t i = 3; i < argc; ++i)
        {
            // Check for duplicate column names.
            auto it = columnNames.find(std::string(argv[i]));
            CHECK_NE(it, columnNames.end())
                << "Found duplicate column name \""
                << argv[i]
                << "\".";

            columnNames.insert(std::string(argv[i]));
        }
    }
    catch (Logging::CheckException e)
    {
        std::cout
            << "Error: "
            << e.GetMessage()
            << std::endl;

        Usage();
        return 0;
    }

    return 1;
}
