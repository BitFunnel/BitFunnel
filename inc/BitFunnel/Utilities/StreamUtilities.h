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

#include <cstddef>  // For std::ptrdiff
#include <iosfwd>
#include <istream>
#include <memory>
#include <ostream>
#include <string>

#include "BitFunnel/Utilities/IInputStream.h"
#include "BitFunnel/Utilities/StandardInputStream.h"
#include "LoggerInterfaces/Logging.h"

namespace BitFunnel
{
    // StreamUtilities mainly defines functions to read/write POD types to and
    // from streams. The read and write functions break stream operations into
    // chunks to avoid the istream/ostream limit on transation sizes.
    namespace StreamUtilities
    {
        //
        // Methods for reading from an input stream.
        //

        // Reads a specified number of bytes from stream into buffer.
        // Internally this function reads in 1GB chunks if byteCount is more than 1G.
        // This function throws exception if read fails.
        void ReadBytes(IInputStream& stream, void* buffer, size_t byteCount);

        // Reads the value of type T from stream.
        // Depending on the size of T, this read variable number of bytes.
        template <typename T>
        T ReadField(IInputStream& stream);

        // Read string from stream.
        void ReadString(IInputStream& stream, std::string& str);

        // Read an array of T from stream.
        template <typename T>
        void ReadArray(IInputStream& stream, T* buffer, size_t itemCount);

        //
        // Wrapper methods for reading from an std::istream.
        //

        // Reads a specified number of bytes from stream into buffer.
        // Internally this function reads in 1GB chunks if byteCount is more than 1G.
        // This function throws exception if read fails.
        void ReadBytes(std::istream& stream, void* buffer, size_t byteCount);

        // Reads the value of type T from stream.
        // Depends on the size of T, this read variable number of bytes.
        template <typename T>
        T ReadField(std::istream& stream);

        // Read string from stream.
        void ReadString(std::istream& stream, std::string& str);

        // Read an array of T from stream.
        template <typename T>
        void ReadArray(std::istream& stream, T* buffer, size_t itemCount);


        //
        // Methods for writing to an output stream.
        //

        // Writes a specified number of bytes in buffer to stream.
        // Internally this function writes in 1GB chunks if byteCount is more than 1G.
        // This function throws exception if write fails.
        void WriteBytes(std::ostream &stream, const char* buffer, size_t byteCount);

        // Writes value of type T to stream.
        // Depends on the size of T, this writes variable number of bytes.
        template <typename T>
        void WriteField(std::ostream& stream, const T& field);

        // Write a string to stream. prefix the string with its length.
        void WriteString(std::ostream& strea, const std::string& str);

        // Writes an array of T to stream.
        template <typename T>
        void WriteArray(std::ostream& stream, const T* buffer, size_t itemCount);

        // The chunk size in bytes to read from istream or write to ostream.
        // The reasons are:
        //  1. istream/ostream does not support int64 length;
        //  2. we can trace the progress if necessary;
        //  3. (suspected but not proved) reduce memory footprint.
        const int c_readWriteChunkSize = (1 << 30);


        //
        // Helper methods.
        //

        // Trim leading/trailing space characters: " \t\r";
        void Trim(std::string& str);

        // Returns a copy of str which has leading/trailing space characters trimmed.
        std::string TrimCopy(const std::string& str);

        // Read a line from stream, and trim leading/trailing space characters: " \t\r";
        void ReadTrimmedLine(std::istream& stream, std::string& str);
    }


    //
    // Implementations of templated methods for reading from an input stream.
    //

    template <typename T>
    T StreamUtilities::ReadField(IInputStream& stream)
    {
        T temp;
        const size_t bytesRead = stream.Read(reinterpret_cast<char*>(&temp), sizeof(T));
        LogAssertB(bytesRead == sizeof(T), "Stream Read Error");
        return temp;
    }


    template <typename T>
    void StreamUtilities::ReadArray(IInputStream& stream,
                                    T* buffer,
                                    size_t itemCount)
    {
        std::ptrdiff_t byteCount = itemCount * sizeof(T);
        ReadBytes(stream, reinterpret_cast<char*>(buffer), byteCount);
    }


    //
    // Implementations of templated wrapper methods for reading from an std::istream.
    //

    template <typename T>
    T StreamUtilities::ReadField(std::istream& stream)
    {
        StandardInputStream stdStream(stream);

        return ReadField<T>(stdStream);
    }


    template <typename T>
    void StreamUtilities::ReadArray(std::istream& stream,
                                    T* buffer,
                                    size_t itemCount)
    {
        StandardInputStream stdStream(stream);

        ReadArray<T>(stdStream, buffer, itemCount);
    }


    //
    // Implementations for methods for writing to an output stream.
    //

    template <typename T>
    void StreamUtilities::WriteField(std::ostream& stream, const T& field)
    {
        stream.write(reinterpret_cast<const char*>(&field), sizeof(T));
        LogAssertB(!stream.bad(), "Stream Write Error");
    }


    template <typename T>
    void StreamUtilities::WriteArray(std::ostream& stream,
                                     const T* buffer,
                                     size_t itemCount)
    {
        WriteBytes(stream, reinterpret_cast<const char*>(buffer),
                   itemCount * sizeof(T));
    }
}
