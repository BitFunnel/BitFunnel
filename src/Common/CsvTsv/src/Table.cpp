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


#include <string.h> // For strdup. TODO: don't use strdup.

#include <sstream>

#include "CsvTsv/ParseError.h"
#include "CsvTsv/Table.h"

#ifdef _MSC_VER
// Use cross platform strdup for simplicity.
#pragma warning(disable:4996)
#endif


namespace CsvTsv
{
    //*************************************************************************
    //
    // ColumnBase<T>
    //
    //*************************************************************************
    ColumnBase::ColumnBase(const char* name, const char* helpMessage)
        : m_name(strdup(name)), m_helpMessage(strdup(helpMessage))
    {
    }


    ColumnBase::~ColumnBase()
    {
        free(m_name);
        free(m_helpMessage);
    }


    const char* ColumnBase::Name() const
    {
        return m_name;
    }


    const char* ColumnBase::HelpMessage() const
    {
        return m_helpMessage;
    }


    //*************************************************************************
    //
    // TypedColumn<T>
    //
    //*************************************************************************
    template <>
    void TypedColumn<int>::SetHexMode(bool hexMode)
    {
        m_hexMode = hexMode;
    }


    template <>
    void TypedColumn<unsigned int>::SetHexMode(bool hexMode)
    {
        m_hexMode = hexMode;
    }


    template <>
    void TypedColumn<long long>::SetHexMode(bool hexMode)
    {
        m_hexMode = hexMode;
    }


    template <>
    void TypedColumn<unsigned long long>::SetHexMode(bool hexMode)
    {
        m_hexMode = hexMode;
    }


    template <>
    void TypedColumn<std::string>::Read(IFieldParser& parser)
    {
        if (!parser.TryReadField(m_value))
        {
            ParseErrorBuilder error(parser);
            error << "syntax error in " << Name() << " column.";
            error.Throw();
        }
        else if (m_value.length() > 0 || !HasInitialValue())
        {
            // The field contains some characters or it is empty and the
            // column has no default value.
            m_hasValue = true;
        }
        else
        {
            // The field is empty and the column has a default value which
            // we can use.
            Reset();
        }
    }


    template <>
    void TypedColumn<int>::Read(IFieldParser& parser)
    {
        std::string s;
        if (parser.TryReadField(s))
        {
            std::stringstream ss(s);
            if (m_hexMode)
            {
                // DESIGN NOTE: the two lines below exhibit different behavior
                //   (1) ss >> std::hex >> m_value;
                //   (2) ss >> std::hex; ss >> m_value;
                // DESIGN NOTE: Operator >> fails for large negative numbers.
                // Work around is to store value into an unsigned int and then
                // cast to int.
                unsigned x;
                ss >> std::hex >> x;
                m_value = static_cast<int>(x);
            }
            else
            {
                ss >> m_value;
            }
            if (ss.fail())
            {
                if (s.length() > 0)
                {
                    // There was actually text here that didn't parse so report an error.
                    ParseErrorBuilder error(parser);
                    error << "expected integer value in " << Name() << " column.";
                    error.Throw();
                }
                else
                {
                    // Field was empty - use default value if it exists.
                    Reset();
                }
            }
            else
            {
                m_hasValue = true;
            }
        }
    }


    template <>
    void TypedColumn<unsigned int>::Read(IFieldParser& parser)
    {
        std::string s;
        if (parser.TryReadField(s))
        {
            std::stringstream ss(s);
            if (m_hexMode)
            {
                // DESIGN NOTE: the two lines below exhibit different behavior
                //   (1) ss >> std::hex >> m_value;
                //   (2) ss >> std::hex; ss >> m_value;
                ss >> std::hex >> m_value;
            }
            else
            {
                ss >> m_value;
            }
            if (ss.fail())
            {
                // TODO: Add code to reject negative numbers.
                // Currently stringstream::operator>>(unsigned int) accepts
                // negative numbers.
                if (s.length() > 0)
                {
                    // There was actually text here that didn't parse so report an error.
                    ParseErrorBuilder error(parser);
                    error << "expected unsigned integer value in " << Name() << " column.";
                    error.Throw();
                }
                else
                {
                    // Field was empty - use default value if it exists.
                    Reset();
                }
            }
            else
            {
                m_hasValue = true;
            }
        }
    }


    template <>
    void TypedColumn<unsigned long int>::Read(IFieldParser& parser)
    {
        std::string s;
        if (parser.TryReadField(s))
        {
            std::stringstream ss(s);
            if (m_hexMode)
            {
                // DESIGN NOTE: the two lines below exhibit different behavior
                //   (1) ss >> std::hex >> m_value;
                //   (2) ss >> std::hex; ss >> m_value;
                ss >> std::hex >> m_value;
            }
            else
            {
                ss >> m_value;
            }
            if (ss.fail())
            {
                // TODO: Add code to reject negative numbers.
                // Currently stringstream::operator>>(unsigned long long int)
                // accepts negative numbers.
                if (s.length() > 0)
                {
                    // There was actually text here that didn't parse so report an error.
                    ParseErrorBuilder error(parser);
                    error << "expected unsigned long integer value in " << Name() << " column.";
                    error.Throw();
                }
                else
                {
                    // Field was empty - use default value if it exists.
                    Reset();
                }
            }
            else
            {
                m_hasValue = true;
            }
        }
    }


    template <>
    void TypedColumn<long long int>::Read(IFieldParser& parser)
    {
        std::string s;
        if (parser.TryReadField(s))
        {
            std::stringstream ss(s);
            if (m_hexMode)
            {
                // DESIGN NOTE: the two lines below exhibit different behavior
                //   (1) ss >> std::hex >> m_value;
                //   (2) ss >> std::hex; ss >> m_value;
                // DESIGN NOTE: Operator >> fails for large negative numbers.
                // Work around is to store value into an unsigned long long
                // and then cast to long long.
                unsigned long long x;
                ss >> std::hex >> x;
                m_value = static_cast<long long>(x);
            }
            else
            {
                ss >> m_value;
            }
            if (ss.fail())
            {
                if (s.length() > 0)
                {
                    // There was actually text here that didn't parse so report an error.
                    ParseErrorBuilder error(parser);
                    error << "expected 64-bit integer value in " << Name() << " column.";
                    error.Throw();
                }
                else
                {
                    // Field was empty - use default value if it exists.
                    Reset();
                }
            }
            else
            {
                m_hasValue = true;
            }
        }
    }


    template <>
    void TypedColumn<unsigned long long int>::Read(IFieldParser& parser)
    {
        std::string s;
        if (parser.TryReadField(s))
        {
            std::stringstream ss(s);
            if (m_hexMode)
            {
                // DESIGN NOTE: the two lines below exhibit different behavior
                //   (1) ss >> std::hex >> m_value;
                //   (2) ss >> std::hex; ss >> m_value;
                ss >> std::hex >> m_value;
            }
            else
            {
                ss >> m_value;
            }
            if (ss.fail())
            {
                // TODO: Add code to reject negative numbers.
                // Currently stringstream::operator>>(unsigned long long int)
                // accepts negative numbers.
                if (s.length() > 0)
                {
                    // There was actually text here that didn't parse so report an error.
                    ParseErrorBuilder error(parser);
                    error << "expected unsigned long long integer value in " << Name() << " column.";
                    error.Throw();
                }
                else
                {
                    // Field was empty - use default value if it exists.
                    Reset();
                }
            }
            else
            {
                m_hasValue = true;
            }
        }
    }


    template <>
    void TypedColumn<double>::Read(IFieldParser& parser)
    {
        std::string s;
        if (parser.TryReadField(s))
        {
            std::stringstream ss(s);
            if ((ss >> m_value).fail())
            {
                if (s.length() > 0)
                {
                    // There was actually text here that didn't parse so report an error.
                    ParseErrorBuilder error(parser);
                    error << "expected floating point value in " << Name() << " column.";
                    error.Throw();
                }
                else
                {
                    // Field was empty - use default value if it exists.
                    Reset();
                }
            }
            else
            {
                m_hasValue = true;
            }
        }
    }


    template <>
    void TypedColumn<bool>::Read(IFieldParser& parser)
    {
        std::string s;
        if (parser.TryReadField(s))
        {
            if (s.compare("TRUE") == 0)
            {
                SetValue(true);
            }
            else if (s.compare("FALSE") == 0)
            {
                SetValue(false);
            }
            else if (s.length() > 0)
            {
                ParseErrorBuilder error(parser);
                error << "expected boolean value (TRUE/FALSE) in " << Name() << " column.";
                error.Throw();
            }
            else
            {
                // Field was empty - use default value if it exists.
                Reset();
            }
        }
    }


    template <>
    bool TypedColumn<std::string>::operator==(const std::string& value) const
    {
        return !strcmp(m_value.c_str(), value.c_str());
    }


    //*************************************************************************
    //
    // TableReader
    //
    //*************************************************************************
    TableReader::TableReader(ITableParser& parser)
        : m_parser(parser)
    {
    }


    void TableReader::DefineColumn(Column& column)
    {
        m_knownColumns.push_back(&column);
    }


    bool TableReader::AtEOF() const
    {
        // Skip over comments.
        std::string comment;
        while (m_parser.ReadComment(comment));

        return m_parser.AtEOF();
    }


    unsigned TableReader::GetColumnCount() const
    {
        // TODO: Should this API return size_t?
        return static_cast<unsigned>(m_columnsInTable.size());
    }


    const Column& TableReader::GetColumn(unsigned i) const
    {
        return *m_columnsInTable[i];
    }


    bool TableReader::Contains(const char* name) const
    {
        for (unsigned j = 0; j < m_columnsInTable.size(); ++j)
        {
            if (!strcmp(m_columnsInTable[j]->Name(), name))
            {
                return true;
            }
        }
        return false;
    }


    bool TableReader::Contains(const Column* column) const
    {
        for (unsigned j = 0; j < m_columnsInTable.size(); ++j)
        {
            if (column == m_columnsInTable[j])
            {
                return true;
            }
        }
        return false;
    }


    void TableReader::ReadPrologue()
    {
        std::vector<std::string> columns;
        m_parser.ReadPrologue(columns);

        for (unsigned c = 0; c < columns.size(); ++c)
        {
            std::string& name = columns[c];
            unsigned i = 0;
            for (i = 0; i < m_knownColumns.size(); ++i)
            {
                if (name.compare(m_knownColumns[i]->Name()) == 0)
                {
                    // Name matches one of the known columns.
                    // Check whether we've already seen this column in the input stream.
                    if (!Contains(name.c_str()))
                    {
                        m_columnsInTable.push_back(m_knownColumns[i]);
                    }
                    else
                    {
                        ParseErrorBuilder error(m_parser);
                        error << "duplicated column name: " << name;
                        error.Throw();
                    }
                    break;
                }
            }
            if (i == m_knownColumns.size())
            {
                ParseErrorBuilder error(m_parser);
                error << "unknown column name: " << name;
                error.Throw();
            }
        }
    }


    void TableReader::ReadEpilogue()
    {
        m_parser.ReadEpilogue();
    }


    void TableReader::ReadDataRow()
    {
        // DESIGN NOTE: ReadDataRow() should not attempt to strip off comments
        // here. The reason is that ReadDataRow() returns void so it cannot
        // handle the case where there is not data left in the file. We must
        // require that ReadDataRow() only be called when the stream is
        // positioned at the beginning of a valid data row. This implies that
        // comments must be skipped by the caller before invoking
        // ReadDataRow(). Comments are currently filtered by the AtEOF()
        // method().

        // Set up initial values in all columns that have them. Remove values from other columns.
        for (unsigned i = 0 ; i < m_knownColumns.size(); ++i)
        {
            m_knownColumns[i]->Reset();
        }

        // TODO: Check for ReadHeader()? Probably not if user can add columns manually.
        for (unsigned i = 0 ; i < m_columnsInTable.size(); ++i)
        {
            Column* column = m_columnsInTable[i];
            if (column->IsInputColumn())
            {
                column->Read(m_parser);
            }
            else
            {
                // TODO: Remove this TODO comment after following code is verified correct. Original code always did skip.
                if (m_parser.AtEOL())
                {
                    m_parser.TrySkipField();
                }
                else
                {
                    column->Read(m_parser);
                }
            }
        }

        if (!m_parser.AtEOL())
        {
            ParseErrorBuilder error(m_parser);
            error << "expected end of line";
            error.Throw();
        }

        // TODO: apply column constraints here. Optional columns, range checks, column relations, etc.
        m_parser.AdvanceToNextRow();
    }


    //*************************************************************************
    //
    // TableWriter
    //
    //*************************************************************************
    TableWriter::TableWriter(ITableFormatter& formatter)
        : m_formatter(formatter)
    {
    }


    void TableWriter::DefineColumn(const Column& column)
    {
        m_columns.push_back(&column);
    }


    void TableWriter::WritePrologue()
    {
        std::vector<const char*> columns;
        for (unsigned i = 0; i < m_columns.size(); ++i)
        {
            columns.push_back(m_columns[i]->Name());
        }
        m_formatter.WritePrologue(columns);
    }


    void TableWriter::WriteDataRow()
    {
        for (unsigned i = 0; i < m_columns.size(); ++i)
        {
            m_columns[i]->Write(m_formatter);
        }
        m_formatter.WriteRowEnd();
    }


    void TableWriter::WriteEpilogue()
    {
        m_formatter.WriteEpilogue();
    }


    //*************************************************************************
    //
    // Table
    //
    //*************************************************************************
    void VectorTable::DefineColumn(VectorColumnBase& column)
    {
        m_columns.push_back(&column);
    }


    void VectorTable::Read(ITableParser& parser)
    {
        TableReader reader(parser);
        for (unsigned i = 0; i < m_columns.size(); ++i)
        {
            reader.DefineColumn(*m_columns[i]);
            m_columns[i]->Clear();
        }

        reader.ReadPrologue();
        while (!reader.AtEOF())
        {
            reader.ReadDataRow();
            for (unsigned i = 0; i < m_columns.size(); ++i)
            {
                m_columns[i]->PushBack();
            }
        }
        reader.ReadEpilogue();
    }


    void VectorTable::Write(ITableFormatter& formatter)
    {
        TableWriter writer(formatter);

        if (m_columns.size() > 0)
        {
            size_t columnLength = m_columns[0]->GetSize();
            for (unsigned i = 0; i < m_columns.size(); ++i)
            {
                if (m_columns[i]->GetSize() != columnLength)
                {
                    Error error("Column length mismatch.");
                    throw error;
                }
                writer.DefineColumn(*m_columns[i]);
                m_columns[i]->ResetWriteCursor();
            }
        }

        writer.WritePrologue();
        while (AdvanceWriteCursors())
        {
            writer.WriteDataRow();
        }
        writer.WriteEpilogue();
    }


    bool VectorTable::AdvanceWriteCursors()
    {
        bool success = false;

        if (m_columns.size() > 0)
        {
            success = true;

            for (unsigned i = 0; i < m_columns.size(); ++i)
            {
                success &= m_columns[i]->AdvanceWriteCursor();
            }
        }

        return success;
    }
}
