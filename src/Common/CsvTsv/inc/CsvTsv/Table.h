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
#include <ostream>
#include <vector>

// C4505 must be enabled for the entire header file and everything that comes 
// after because the compiler generates C4505 after parsing all files.
#pragma warning(disable:4505)


namespace CsvTsv
{
    class Column;
    class ITableParser;
    class ITableFormatter;
    class VectorColumnBase;


    //*************************************************************************
    //
    // VectorTable
    //
    //*************************************************************************
    class VectorTable
    {
    public:
        void DefineColumn(VectorColumnBase& column);

        void Read(ITableParser& parser);
        void Write(ITableFormatter& formatter);

    private:
        bool AdvanceWriteCursors();

        std::vector<VectorColumnBase*> m_columns;
    };


    //*************************************************************************
    //
    // TableReader
    //
    //*************************************************************************
    class TableReader
    {
    public:
        TableReader(ITableParser& parser);

        // Defines an optional column that may be in the file.A subsequent call
        // to ReadPrologue() will detect all columns used in the file.
        void DefineColumn(Column& column);

        // Reads the prologue portion of the file. The prologue portion comes
        // before the data rows and typically contains the names of the
        // columns in the file.
        void ReadPrologue();

        // Reads a row of data, starting at the current stream position. Note
        // that this function does not strip off comments. The stream must be
        // positioned at the beginning of a valid row.
        void ReadDataRow();

        // Reas the epilogue portion of the file. CSV and TSV files don't have
        // epilogues. Formats like XML will typically have an epilogue that
        // contains a closing tag.
        void ReadEpilogue();

        bool AtEOF() const;

        unsigned GetColumnCount() const;
        const Column& GetColumn(unsigned i) const;

        bool Contains(const Column* column) const;
        bool Contains(const char* name) const;

    private:
        // Private assignment operator because this class is non-copyable.
        TableReader& operator=(const TableReader&);

        ITableParser& m_parser;

        std::vector<Column*> m_knownColumns;
        std::vector<Column*> m_columnsInTable;
    };


    //*************************************************************************
    //
    // TableWriter
    //
    //*************************************************************************
    class TableWriter
    {
    public:
        TableWriter(ITableFormatter& formatter);

        void DefineColumn(const Column& column);

        void WritePrologue();
        void WriteDataRow();
        void WriteEpilogue();

    private:
        // Private assignment operator because this class is non-copyable.
        TableWriter& operator=(const TableWriter&);

        ITableFormatter& m_formatter;
        std::vector<const Column*> m_columns;
    };


    //*************************************************************************
    //
    // IFieldParser
    //
    // Provides row parsing services to ITableReader. Responsible for reading 
    // string valued fields from a stream and providing error reporting
    // functionality.
    //
    // Each file format, whether plaintext or binary, should define a class
    // that implements IFieldParser.
    //*************************************************************************
    class IFieldParser
    {
    public:
        virtual ~IFieldParser() {};

        // Attempt to read the next field.
        // If successful, the value of the field will be stored in value and
        // the function will return true. On return the current position will
        // be immediately after the field just returned.
        virtual bool TryReadField(std::string& value) = 0;

        virtual bool TrySkipField() = 0;

        // Returns the current character position relative to beginning of
        // current line.
        virtual unsigned GetPositionInLine() const = 0;

        // Returns the current field position on the current line.
        virtual unsigned GetFieldNumber() const = 0;

        // Returns the current line number.
        virtual unsigned GetRowNumber() const = 0;
    };

    //*************************************************************************
    //
    // ITableParser
    //
    // Provides field and row parsing services to ITableReader.
    //
    // Each file format, whether plaintext or binary, should define a class
    // that implements ITableParser.
    //*************************************************************************
    class ITableParser : public IFieldParser
    {
    public:
        virtual ~ITableParser() {};

        // Returns true if the current position is immediately before the end
        // of a line (or the end of file).
        virtual bool AtEOL() const = 0;

        // Returns true if the current position is at the end of the stream.
        virtual bool AtEOF() const = 0;

        // Reads the prologue portion of the file. This is the portion before
        // the data rows. The prologue contains a header with column names.
        // These names are extracted and added to the columns vector parameter.
        virtual void ReadPrologue(std::vector<std::string>& columns) = 0;

        // Reads the portion of the file after the data rows.
        virtual void ReadEpilogue() = 0;

        // Attempts to read a comment at the current position. If successful,
        // the entire comment (including special comment delimiters) is stored
        // in the comment parameter and the method returns true and the current
        // position is immediately after the comment. Otherwise the method
        // returns false and the current position is unchanged.
        virtual bool ReadComment(std::string& comment) = 0;

        // Moves the current position past the next field on the line if there
        // are fields remaining. Does not advance to the next line. Returns
        // true if the current position after the advance attempt is 
        // immediately before the end of a line.
        virtual bool TrySkipField() = 0;

        // Attempts to move the current position to the beginning of the next
        // line, skipping over any remaining fields on the current line.
        // Returns true if the current position after the advance attempt is at
        // the end of the stream.
        virtual bool AdvanceToNextRow() = 0;
    };

    //*************************************************************************
    //
    // IFieldFormatter
    //
    // Provides field-level formatting services to ITableWriter.
    // Each file format, whether plain-text or binary should define a class
    // that implements IFieldFormatter.
    //*************************************************************************
    class IFieldFormatter
    {
    public:
        virtual ~IFieldFormatter() {};

        // When in hex mode, fields of type int, unsigned int, long, and
        // unsigned long long will be formatted as hexidecimal. This formatting
        // will not include a base prefix (e.g. "0x").
        virtual void SetHexMode(bool hexMode) = 0;

        virtual void WriteField(const std::string& value) = 0;
        virtual void WriteField(const char* value) = 0;
        virtual void WriteField(int value) = 0;
        virtual void WriteField(unsigned int value) = 0;
        virtual void WriteField(long long int value) = 0;
        virtual void WriteField(long long unsigned int value) = 0;
        virtual void WriteField(double value) = 0;
        virtual void WriteField(bool value) = 0;

        virtual void WriteEmptyField() = 0;
    };

    //*************************************************************************
    //
    // ITableFormatter
    //
    // Provides field and row level formatting services to ITableWriter.
    // Each file format, whether plain-text or binary should define a class
    // that implements ITableFormatter.
    //*************************************************************************
    class ITableFormatter : public IFieldFormatter
    {
    public:
        virtual ~ITableFormatter() {};

        virtual void WritePrologue(const std::vector<const char*>& columns) = 0;
        virtual void WriteRowEnd() = 0;
        virtual void WriteEpilogue() = 0;
    };


    class Column
    {
    public:
        virtual ~Column() {};

        virtual const char* Name() const = 0;
        virtual const char* HelpMessage() const = 0;

        virtual void Read(IFieldParser& parser) = 0;
        virtual void Write(IFieldFormatter& formatter) const = 0;

        virtual bool IsInputColumn() const = 0;
        virtual bool IsOutputColumn() const = 0;

        virtual void Reset() = 0;
    };


    //*************************************************************************
    //
    // ColumnBase
    //
    // Abstract base class that unifies all of the the variants of TypedColumn<T>.
    //*************************************************************************
    class ColumnBase : public virtual Column
    {
    public:
        ~ColumnBase();

        //
        // Column methods
        //
        const char* Name() const;
        const char* HelpMessage() const;

    protected:
         ColumnBase(const char* name, const char* helpMessage);

    private:
        char* m_name;
        char* m_helpMessage;
    };

    //*************************************************************************
    //
    // TypedColumn<T>
    //
    //*************************************************************************
    //  TODO: add concept of default value for columns?
    template <class T> 
    class TypedColumn : public ColumnBase
    {
    public:
        TypedColumn(const char* name, const char* helpMessage);
        TypedColumn(const char* name, const char* helpMessage, T initialValue);

        // When in hex mode, columns of type int, unsigned int, long, and
        // unsigned long long will be formatted as hexidecimal. This formatting
        // will not include a base prefix (e.g. "0x").
        void SetHexMode(bool hexMode);

        bool HasValue() const;

        const T& GetValue() const;
        operator T() const;

        void SetValue(const T& value);
        const T& operator=(const T& value);

        bool operator==(const T& value) const;

        //
        // ColumnBase methods
        //
        bool HasInitialValue() const;
        const T& InitialValue() const;

        void Read(IFieldParser& parser);
        void Write(IFieldFormatter& formatter) const;

        void Reset();

        // TODO: add a method to get type name to use for error messages (e.g.
        // expected an integer)

    private:
        bool m_hasValue;
        T m_value;

        bool m_hasInitialValue;
        T m_initialValue;

        bool m_hexMode;
    };

    //*************************************************************************
    //
    // InputColumn<T>
    //
    //*************************************************************************
    template <class T>
    class InputColumn : public TypedColumn<T>
    {
    public:
        InputColumn(const char* name, const char* helpMessage);
        InputColumn(const char* name, const char* helpMessage, T initialValue);

        bool IsInputColumn() const;
        bool IsOutputColumn() const;

        using TypedColumn<T>::operator=;
    };

    //*************************************************************************
    //
    // OutputColumn<T>
    //
    //*************************************************************************
    template <class T>
    class OutputColumn : public TypedColumn<T>
    {
    public:
        OutputColumn(const char* name, const char* helpMessage);
        OutputColumn(const char* name, const char* helpMessage, T initialValue);

        bool IsInputColumn() const;
        bool IsOutputColumn() const;

        using TypedColumn<T>::operator=;
    };


    //*************************************************************************
    //
    // VectorColumnBase
    //
    //*************************************************************************
    class VectorColumnBase : public virtual Column
    {
    public:
        virtual void Clear() = 0;
        virtual void PushBack() = 0;

        virtual void ResetWriteCursor() = 0;
        virtual bool AdvanceWriteCursor() = 0;

        virtual size_t GetSize() const = 0;
    };


    //*************************************************************************
    //
    // TypedVectorColumn<T>
    //
    //*************************************************************************
#pragma warning(push)
#pragma warning(disable:4250)
    template <class T>
    class TypedVectorColumn : public TypedColumn<T>, public VectorColumnBase
    {
    public:
        TypedVectorColumn(const char* name, const char* helpMessage, std::vector<T>& data);
        TypedVectorColumn(const char* name, const char* helpMessage, T initialValue, std::vector<T>& data);

        using TypedColumn<T>::operator=;

        //
        // VectorColumnBase methods.
        //
        void Clear();
        void PushBack();

        void ResetWriteCursor();
        bool AdvanceWriteCursor();

        size_t GetSize() const;

    private:
        // DESIGN NOTE: Cursor is int, rather than unsigned because it uses -1
        // to encode the Clear() state.
        int m_cursor;
        std::vector<T>& m_data;
    };
#pragma warning(pop)


    //*************************************************************************
    //
    // InputVectorColumn<T>
    //
    //*************************************************************************
#pragma warning(push)
#pragma warning(disable:4250)
    template <class T>
    class InputVectorColumn : public TypedVectorColumn<T>
    {
    public:
        InputVectorColumn(const char* name, const char* helpMessage, std::vector<T>& data);
        InputVectorColumn(const char* name, const char* helpMessage, T initialValue, std::vector<T>& data);

        using TypedColumn<T>::operator=;

        bool IsInputColumn() const;
        bool IsOutputColumn() const;
    };
#pragma warning(pop)


    //*************************************************************************
    //
    // OutputVectorColumn<T>
    //
    //*************************************************************************
#pragma warning(push)
#pragma warning(disable:4250)
    template <class T>
    class OutputVectorColumn : public TypedVectorColumn<T>
    {
    public:
        OutputVectorColumn(const char* name, const char* helpMessage, std::vector<T>& data);
        OutputVectorColumn(const char* name, const char* helpMessage, T initialValue, std::vector<T>& data);

        using TypedColumn<T>::operator=;

        bool IsInputColumn() const;
        bool IsOutputColumn() const;
    };
#pragma warning(pop)


    //*************************************************************************
    //
    // Definitions for template functions after this point.
    //
    //*************************************************************************

    //*************************************************************************
    //
    // Definitions for TypedColumn<T>
    //
    //*************************************************************************
    template <class T>
    TypedColumn<T>::TypedColumn(const char* name, const char* helpMessage)
        : ColumnBase(name, helpMessage),    
          m_hasValue(false), m_hasInitialValue(false), m_hexMode(false)
    {
    }


    template <class T>
    TypedColumn<T>::TypedColumn(const char* name, const char* helpMessage, T initialValue)
        : ColumnBase(name, helpMessage),
          m_hasValue(true), m_value(initialValue),
          m_hasInitialValue(true), m_initialValue(initialValue), m_hexMode(false)
    {
    }


    template <class T>
    bool TypedColumn<T>::HasValue() const
    {
        return m_hasValue;
    }


    template <class T>
    const T& TypedColumn<T>::GetValue() const
    {
        if (!HasValue())
        {
            throw std::runtime_error("Attempting to get uninitialized value.");
        }
        return m_value;
    }


    template <class T>
    TypedColumn<T>::operator T() const
    {
        return GetValue();
    }


    template <class T>
    void TypedColumn<T>::SetValue(const T& value)
    {
        m_value = value;
        m_hasValue = true;
    }


    template <class T>
    const T& TypedColumn<T>::operator=(const T& value)
    {
        SetValue(value);
        return value;
    }


    template <class T>
    bool TypedColumn<T>::operator==(const T& value) const
    {
        return m_value == value;
    }


    template <class T>
    bool TypedColumn<T>::HasInitialValue() const
    {
        return m_hasInitialValue;
    }


    template <class T>
    const T& TypedColumn<T>::InitialValue() const
    {
        if (!HasInitialValue())
        {
            throw std::runtime_error("Attempting to get uninitialized initial value.");
        }
        return m_initialValue;
    }


    template <class T>
    void TypedColumn<T>::Write(IFieldFormatter& formatter) const
    {
        if (HasValue())
        {
            formatter.SetHexMode(m_hexMode);
            formatter.WriteField(GetValue());
        }
        else
        {
            formatter.WriteEmptyField();
        }
    }


    template <class T>
    void TypedColumn<T>::Reset()
    {
        if (HasInitialValue())
        {
            SetValue(InitialValue());
        }
        else
        {
            m_hasValue = false;
        }
    }


    //*************************************************************************
    //
    // Definitions for InputColumn<T>
    //
    //*************************************************************************
    template <class T>
    InputColumn<T>::InputColumn(const char* name, const char* helpMessage)
        : TypedColumn<T>(name, helpMessage)
    {
    }

    template <class T>
    InputColumn<T>::InputColumn(const char* name, const char* helpMessage, T initialValue)
        : TypedColumn<T>(name, helpMessage, initialValue)
    {
    }


    template <class T>
    bool InputColumn<T>::IsInputColumn() const
    {
        return true;
    }


    template <class T>
    bool InputColumn<T>::IsOutputColumn() const
    {
        return false;
    }


    //*************************************************************************
    //
    // Definitions for OutputColumn<T>
    //
    //*************************************************************************
    template <class T>
    OutputColumn<T>::OutputColumn(const char* name, const char* helpMessage)
        : TypedColumn<T>(name, helpMessage)
    {
    }

    template <class T>
    OutputColumn<T>::OutputColumn(const char* name, const char* helpMessage, T initialValue)
        : TypedColumn<T>(name, helpMessage, initialValue)
    {
    }


    template <class T>
    bool OutputColumn<T>::IsInputColumn() const
    {
        return false;
    }


    template <class T>
    bool OutputColumn<T>::IsOutputColumn() const
    {
        return true;
    }


    //*************************************************************************
    //
    // Definitions for TypedVectorColumn<T>
    //
    //*************************************************************************
    template <class T>
    TypedVectorColumn<T>::TypedVectorColumn(const char* name,
                                            const char* helpMessage,
                                            std::vector<T>& data)
        : TypedColumn(name, helpMessage),
          m_data(data)
    {
        ResetWriteCursor();
    }


    template <class T>
    TypedVectorColumn<T>::TypedVectorColumn(const char* name,
                                            const char* helpMessage,
                                            T initialValue,
                                            std::vector<T>& data)
        : TypedColumn(name, helpMessage, initialValue),
          m_data(data)
    {
        ResetWriteCursor();
    }


    template <class T>
    void TypedVectorColumn<T>::Clear()
    {
        m_data.clear();
    }


    template <class T>
    void TypedVectorColumn<T>::PushBack()
    {
        m_data.push_back(GetValue());
    }


    template <class T>
    void TypedVectorColumn<T>::ResetWriteCursor()
    {
        m_cursor = -1;
    }


    template <class T>
    bool TypedVectorColumn<T>::AdvanceWriteCursor()
    {
        bool success = false;

        if (m_cursor + 1 < m_data.size())
        {
            ++m_cursor;
            SetValue(m_data[m_cursor]);
            success = true;
        }

        return success;
    }


   template <class T>
   size_t TypedVectorColumn<T>::GetSize() const
   {
       return m_data.size();
   }

    //*************************************************************************
    //
    // Definitions for InputVectorColumn<T>
    //
    //*************************************************************************
    template <class T>
    InputVectorColumn<T>::InputVectorColumn(const char* name,
                                            const char* helpMessage,
                                            std::vector<T>& data)
        : TypedVectorColumn<T>(name, helpMessage, data)
    {
    }

    template <class T>
    InputVectorColumn<T>::InputVectorColumn(const char* name,
                                            const char* helpMessage,
                                            T initialValue,
                                            std::vector<T>& data)
        : TypedVectorColumn<T>(name, helpMessage, initialValue, data)
    {
    }


    template <class T>
    bool InputVectorColumn<T>::IsInputColumn() const
    {
        return true;
    }


    template <class T>
    bool InputVectorColumn<T>::IsOutputColumn() const
    {
        return false;
    }


    //*************************************************************************
    //
    // Definitions for OutputColumn<T>
    //
    //*************************************************************************
    template <class T>
    OutputVectorColumn<T>::OutputVectorColumn(const char* name,
                                              const char* helpMessage,
                                              std::vector<T>& data)
        : TypedVectorColumn<T>(name, helpMessage, data)
    {
    }

    template <class T>
    OutputVectorColumn<T>::OutputVectorColumn(const char* name,
                                              const char* helpMessage,
                                              T initialValue,
                                              std::vector<T>& data)
        : TypedVectorColumn<T>(name, helpMessage, initialValue, data)
    {
    }


    template <class T>
    bool OutputVectorColumn<T>::IsInputColumn() const
    {
        return false;
    }


    template <class T>
    bool OutputVectorColumn<T>::IsOutputColumn() const
    {
        return true;
    }
}

