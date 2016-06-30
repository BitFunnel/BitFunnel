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

#include "CsvTsv/Csv.h"

#include <cstring>

namespace CsvTsv
{
    const char* c_headerCommentText = "#Fields:";

    //*************************************************************************
    //
    // CsvTableParser
    //
    //*************************************************************************
    CsvTableParser::CsvTableParser(std::istream& input)
        : DelimitedTableParser(input, ','),
          m_headerCommentMode(false)
    {
    }


    CsvTableParser::~CsvTableParser()
    {
    }


    void CsvTableParser::SetHeaderCommentMode(bool mode)
    {
        m_headerCommentMode = mode;
    }


    void CsvTableParser::ReadPrologue(std::vector<std::string>& columns)
    {
        if (m_headerCommentMode)
        {
            // Skip over comments until field definition comment is found.
            // Then extract the column names from the field definition comment.
            std::string comment;
            while (ReadComment(comment))
            {
                if (m_headerCommentMode && comment.find(c_headerCommentText) == 0)
                {
                    const char* current = comment.c_str() + strlen(c_headerCommentText);
                    ReadColumnNames(current, columns);

                    // Now that we've found the header comment, we've completed
                    // parsing the prologue.
                    return;
                }
            }

            // We didn't find the expected header comment.
            ParseErrorBuilder error(*this);
            error << "Expected a field definition comment starting with \"";
            error << c_headerCommentText << "\".";
            error.Throw();
        }
        else
        {
            // Skip over comments until first non-comment row.
            std::string comment;
            while (ReadComment(comment));

            // Read in the header definition row.
            std::string header;
            ReadLine(header);
            ReadColumnNames(header.c_str(), columns);
        }
    }


    bool CsvTableParser::ReadComment(std::string& comment)
    {
        comment.clear();

        // Comments are empty lines and lines that start with '#'.
        if (GetPositionInLine() == 0 && (m_input->peek() == '#' || m_input->peek() == '\n' || m_input->peek() == '\r'))
        {
            ReadLine(comment);
            return true;
        }

        return false;
    }


    // Attempt to parse the next field as a string, int, double or bool.
    // If successful, the value of the field will be stored in value and the function
    // will return true. On return the current position will be immediately after
    // the field just returned.
    bool CsvTableParser::TryReadField(std::string& value)
    {
        value.clear();

        // For performance, ensure that value has enough space that it won't
        // need to reallocate during most invocations.
        value.reserve(100);

        if (m_input->eof())
        {
            return false;
        }
        else
        {
            // TODO: eliminate this cast by changing API to int.
            char c = static_cast<char>(m_input->peek());
            if (c == '"')
            {
                // This field is a quoted string. Skip over the opening quote.
                GetChar();

                for (;;)
                {
                    if (AtEOL())
                    {
                        // TODO: BUGBUG: check the RFC to see if we are allowed to place a newline
                        // inside a quoted field. If this is legal, we will need a different check here
                        // and the concept of lines and rows will not be different.
                        // For now just treat as a parse error.

                        // Error: reached EOL before the closing quote.
                        return false;
                    }
                    else
                    {
                        c = GetChar();
                        if (c == '"')
                        {
                            // We're either at a quoted quote or the end of the string.
                            if (m_input->peek() == '"')
                            {
                                // We're at a quoted quote. Add it to the value string.
                                value.push_back(GetChar());
                            }
                            else
                            {
                                // We just passed the closing quote.
                                // Move past the field delimiter if one exists.
                                if (m_input->peek() == ',')
                                {
                                    GetChar();
                                    return true;
                                }
                                else if (!AtEOL())
                                {
                                    // Error. There was no trailing delimiter and we're not at the end of the line.
                                    return false;
                                }
                                else
                                {
                                    return true;
                                }
                            }
                        }
                        else
                        {
                            value.push_back(c);
                        }
                    }
                }
            }
            else
            {
                // This field is an unquoted string.
                for (;;)
                {
                    if (AtEOL())
                    {
                        // Empty string at the end of a line or the end of the file is ok.
                        break;
                    }
                    else if (m_input->peek() == ',')
                    {
                        // We've reached the next field. Move past the delimiter.
                        GetChar();
                        break;
                    }
                    else
                    {
                        // This character should be appended to the value string.
                        value.push_back(GetChar());
                    }
                }
                return true;
            }
        }
    }


    //*************************************************************************
    //
    // CsvTableFormatter
    //
    //*************************************************************************
    CsvTableFormatter::CsvTableFormatter(std::ostream &out)
        : DelimitedTableFormatter(out, ',', '"'),
          m_headerCommentMode(false)
    {
    }


    void CsvTableFormatter::SetHeaderCommentMode(bool mode)
    {
        m_headerCommentMode = mode;
    }


    void CsvTableFormatter::WritePrologue(const std::vector<const char*>& columns)
    {
        if (m_headerCommentMode)
        {
            *m_out << c_headerCommentText;
        }
        WriteColumns(columns);
    }


    void CsvTableFormatter::WriteEscapedValue(const char* value)
    {
        // This string has characters that must be escaped.
        // Emit opening quote.
        *m_out << '"';

        while (*value != 0)
        {
            *m_out << *value;

            if (*value == '"')
            {
                // Escape the quote with a second quote.
                *m_out << '"';
            }
            else if (*value == '\n' || *value == '\r')
            {
                // TODO: BUGBUG: check the RFC to see if newline characters are allowed
                // in quoted fields. Right now the parser does not allow newlines in quoted fields.
                throw std::runtime_error("Quoted CSV fields cannot contain newline characters.");
            }

            value++;
        }

        // Emit trailing quote.
        *m_out << '"';
    }
}
