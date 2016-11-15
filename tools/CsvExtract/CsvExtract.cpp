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

#include "CsvExtract.h"
#include "CsvTsv/Csv.h"
#include "LoggerInterfaces/Check.h"


namespace BitFunnel
{
    int CsvExtract::Main(std::istream& input,
                         std::ostream& output,
                         int argc,
                         char const * argv[])
    {
        try
        {
            bool plainTextOutput = false;

            std::vector<std::string> columnNames;

            for (size_t i = 1; i < static_cast<size_t>(argc); ++i)
            {
                const std::string arg(argv[i]);

                if (arg == "-plaintext")
                {
                    plainTextOutput = true;
                }
                else
                {
                    // Check for duplicate column names.
                    auto it = std::find(columnNames.begin(),
                                        columnNames.end(),
                                        arg);
                    CHECK_TRUE(it == columnNames.end())
                        << "Found duplicate column name \""
                        << arg
                        << "\".";

                    columnNames.push_back(arg);
                }
            }

            CHECK_GT(columnNames.size(), 0)
                << "No column names specified.";

            Filter(input, output, columnNames, plainTextOutput);
        }
        catch (Logging::CheckException e)
        {
            output
                << "Error: "
                << e.GetMessage()
                << std::endl;

            Usage(output);
            return 0;
        }

        return 1;
    }


    // static
    void CsvExtract::Filter(std::istream& input,
                            std::ostream& output,
                            std::vector<std::string> const & columnNames,
                            bool plainTextOutput)
    {
        CHECK_GT(columnNames.size(), 0u)
            << "Expect at least one column to extract.";

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
                size_t columnIndex = static_cast<size_t>(it - columnNames.begin());
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

        // Now write the new header.
        for (auto column : columnNames)
        {
            formatter.WriteField(column);
        }
        formatter.WriteRowEnd();

        // Then copy filtered rows one by one.
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
                if (plainTextOutput)
                {
                    if (i != 0)
                    {
                        output << ",";
                    }
                    output << inputRow[inputIndex[i]];
                }
                else
                {
                    formatter.WriteField(inputRow[inputIndex[i]]);
                }
            }
            formatter.WriteRowEnd();
        }
    }


    // static
    void CsvExtract::Usage(std::ostream& output)
    {
        output
            << "CsvExtract [-plaintext] [<column number>]+" << std::endl
            << std::endl
            << "  Reads a .csv file from standard input." << std::endl
            << "  Writes filtered .csv file to standard output." << std::endl
            << std::endl
            << "  The -plaintext option causes the output to be" << std::endl
            << "  without csv escaping." << std::endl
            << std::endl;
    }
}
