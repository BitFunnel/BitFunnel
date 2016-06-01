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

#include "CsvTsv/Tsv.h"


namespace CsvTsv
{
    //*************************************************************************
    //
    // TsvTableParser
    //
    //*************************************************************************
    TsvTableParser::TsvTableParser(std::istream& input)
        : DelimitedTableParser(input, '\t')
    {
    }


    TsvTableParser::~TsvTableParser()
    {
    }


    void TsvTableParser::ReadPrologue(std::vector<std::string>& columns)
    {
        // Read in the header definition row.
        std::string header;
        ReadLine(header);
        ReadColumnNames(header.c_str(), columns);
    }


    bool TsvTableParser::ReadComment(std::string& /*comment*/)
    {
        // TSV format doesn't support comments.
        return false;
    }


    // Attempt to parse the next field as a string, int, double or bool.
    // If successful, the value of the field will be stored in value and the
    // function will return true. On return the current position will be
    // immediately after the field just returned.
    bool TsvTableParser::TryReadField(std::string& value)
    {
        value.clear();

        // For performance, ensure that value has enough space that it won't
        // need to reallocate during most invocations.
        value.reserve(100);

        if (AtEOL())
        {
            return false;
        }
        else
        {
            for (;;)
            {
                if (AtEOL())
                {
                    break;
                }
                else
                {
                    char c = GetChar();
                    if (c == '\t')
                    {
                        break;
                    }
                    else
                    {
                        value.push_back(c);
                    }
                }
            }
            return true;
        }
    }

    //*************************************************************************
    //
    // TsvTableFormatter
    //
    //*************************************************************************
    TsvTableFormatter::TsvTableFormatter(std::ostream &out)
        : DelimitedTableFormatter(out, '\t', 0)
    {
    }


    void TsvTableFormatter::WritePrologue(const std::vector<const char*>& columns)
    {
        WriteColumns(columns);
    }



    void TsvTableFormatter::WriteEscapedValue(const char* value)
    {
        std::string escapee;
        if (value != NULL)
        {
            escapee = std::string(value);
        }
        else
        {
            escapee = "";
        }
        throw std::runtime_error("WriteEscapedValue not implemented; failed to escape"
                                 + escapee);
    }
}
