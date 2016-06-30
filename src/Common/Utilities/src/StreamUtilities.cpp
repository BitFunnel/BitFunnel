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


#include <inttypes.h>
#include <fstream>
#include <string>
#include <vector>

#include "BitFunnel/Utilities/StreamUtilities.h"
#include "LoggerInterfaces/Logging.h"


using namespace std;

namespace BitFunnel
{
    void StreamUtilities::ReadBytes(IInputStream &stream, void* buffer,
                                    size_t byteCount)
    {
        LogAssertB(buffer != nullptr, "buffer == nullptr");
        size_t offset = 0;  // number of bytes read.
        while (byteCount > 0)
        {
            size_t len = (byteCount <= c_readWriteChunkSize) ?
                byteCount : c_readWriteChunkSize;
            const size_t bytesRead = stream.Read(
                                     static_cast<char*>(buffer) + offset, len);
            LogAssertB(bytesRead == len, "Stream Read Unexpected Bytes Count");
            byteCount -= len;
            offset += len;
        }
    }


    void StreamUtilities::ReadBytes(std::istream& stream, void* buffer,
                                    size_t byteCount)
    {
        StandardInputStream stdStream(stream);

        ReadBytes(stdStream, buffer, byteCount);
    }


    void StreamUtilities::WriteBytes(std::ostream &stream, const char* buffer,
                                     size_t byteCount)
    {
        LogAssertB(buffer != nullptr, "buffer == nullptr");
        size_t offset = 0;  // number of bytes written.
        while (byteCount > 0)
        {
            size_t len = (byteCount <= c_readWriteChunkSize) ?
                byteCount : c_readWriteChunkSize;
            stream.write(static_cast<const char*>(buffer + offset), len);
            LogAssertB(!stream.bad(), "Stream Write Error");
            byteCount -= len;
            offset += len;
            // Trace progress if we are writing full chunks.
            if (offset > c_readWriteChunkSize)
            {
                LogB(Logging::Info, "StreamIO", "write chunk:%-10lld total:%lld",
                     len, offset);
            }
        }
    }


    void StreamUtilities::ReadString(IInputStream& stream, std::string& str)
    {
        unsigned strLen;
        strLen = ReadField<unsigned>(stream);

        std::vector<char> buffer(strLen);
        if (strLen > 0)
        {
            ReadArray(stream, &buffer.front(), buffer.size());
        }

        str.assign(&buffer.front(), buffer.size());
    }


    void StreamUtilities::ReadString(std::istream& stream, std::string& str)
    {
        StandardInputStream stdStream(stream);

        ReadString(stdStream, str);
    }


    void StreamUtilities::WriteString(std::ostream& stream,
                                      const std::string& str)
    {
        WriteField<unsigned>(stream, static_cast<unsigned>(str.size()));
        WriteBytes(stream, str.c_str(), str.size());
    }


    void StreamUtilities::Trim(std::string& str)
    {
        const std::string c_spaceChars = " \t\r";

        std::string::size_type pos = str.find_last_not_of(c_spaceChars);

        if(pos == std::string::npos)
        {
            str.erase(str.begin(), str.end());
        }
        else
        {
            str.erase(pos + 1);
            pos = str.find_first_not_of(c_spaceChars);
            if(pos != std::string::npos)
            {
                str = str.erase(0, pos);
            }
        }
    }


    std::string StreamUtilities::TrimCopy(const std::string& str)
    {
        std::string s = str;

        Trim(s);

        return s;
    }


    void StreamUtilities::ReadTrimmedLine(std::istream& stream,
                                          std::string& str)
    {
        getline(stream, str);

        if (!stream.eof())
        {
            LogAssertB(!stream.fail(), "Stream.fail()");
        }

        Trim(str);
    }
}
