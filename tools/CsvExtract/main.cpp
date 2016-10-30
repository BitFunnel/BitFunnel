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

#include <algorithm>
#include <iostream>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "CmdLineParser/CmdLineParser.h"
#include "CsvTsv/Csv.h"
#include "LoggerInterfaces/Check.h"


void Filter(std::istream& input,
            std::ostream& output,
            std::vector<std::string> const & columnNames)
{
    CsvTsv::CsvTableParser parser(input);
    CsvTsv::CsvTableFormatter formatter(output);

    // Tracks which members of columnNames were actually found in the header
    // and their positions.
    std::vector<bool> foundColumn(columnNames.size(), false);
    std::vector<size_t> inputIndex(columnNames.size(), 0);

    // For each column position in the input file, track whether to keep the
    // column and reseve space for storing its values.
    std::vector<bool> keepColumn;
    std::vector<std::string> inputRow;

    // Read in the header, identifying those input columns that will be output.
    std::string name;
    while (!parser.AtEOL())
    {
        CHECK_TRUE(parser.TryReadField(name))
            << "Expected field in header.";

        auto it = std::find(columnNames.begin(),
                            columnNames.end(),
                            name);

        if (it == columnNames.end())
        {
            // Discard this column.
            keepColumn.push_back(false);
        }
        else
        {
            // Keep this column, but update its position to reflect the order
            // in columnNames.
            keepColumn.push_back(true);
            size_t columnIndex = it - columnNames.begin();
            inputIndex[columnIndex] = inputRow.size();
            foundColumn[columnIndex] = true;
        }
        inputRow.push_back("");
    }
    parser.AdvanceToNextRow();

    // Ensure all requested columns were found.
    for (size_t i = 0; i < foundColumn.size(); ++i)
    {
        CHECK_TRUE(foundColumn[i])
            << "Input file did not contain column \""
            << columnNames[i]
            << "\".";
    }

    // Now process rows one by one.
    size_t lineNumber = 1;
    while (!parser.AtEOF())
    {
        // Read one row.
        for (size_t i = 0; i < keepColumn.size(); ++i)
        {
            CHECK_TRUE(parser.TryReadField(inputRow[i]))
                << "Missing field on line "
                << lineNumber
                << ", column "
                << i;
        }
        parser.AdvanceToNextRow();
        lineNumber++;

        // Write one row.
        for (size_t i = 0; i < columnNames.size(); ++i)
        {
            formatter.WriteField(inputRow[inputIndex[i]]);
        }
        formatter.WriteRowEnd();
    }
}


void Test1()
{
    std::stringstream input;
    input
        << "a,b,c,d,e\n"
        << "1,2,3,4,5\n"
        << "p,q,r,s,t\n";

    std::vector<std::string> columns;
    columns.push_back("d");
    columns.push_back("b");

    Filter(input, std::cout, columns);
}


int main(int /*argc*/, char** /*argv*/)
{
    Test1();
}


void Usage()
{
    std::cout
        << "CsvExtract <input file> <output file> [<column number>]+" << std::endl;
}


int main2(int argc, char** argv)
{
    try
    {
        CHECK_GT(argc, 1)
            << "Expect [<column number>]+";

        std::vector<std::string> columnNames;

        for (size_t i = 3; i < argc; ++i)
        {
            // Check for duplicate column names.
            auto it = std::find(columnNames.begin(),
                                columnNames.end(),
                                std::string(argv[i]));
            CHECK_TRUE(it == columnNames.end())
                << "Found duplicate column name \""
                << argv[i]
                << "\".";

            columnNames.push_back(std::string(argv[i]));
        }

        Filter(std::cin, std::cout, columnNames);
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
