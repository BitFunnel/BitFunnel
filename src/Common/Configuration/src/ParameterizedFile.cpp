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


#include <fstream>
#include <istream>
#include <Windows.h>                // For DeleteFile.

#include "LoggerInterfaces/Logging.h"
#include "ParameterizedFile.h"


namespace BitFunnel
{
    ParameterizedFile::ParameterizedFile(const char* path,
                                         const char* baseName,
                                         const char* extension)
    {
        // Ensure that the base name does not contain '\' or '.'.
        LogAssertB(strchr(baseName, '\\') == nullptr);
        LogAssertB(strchr(baseName, '.') == nullptr);

        // Ensure that extension starts with '.' and does not contain '\'.
        LogAssertB(extension[0] == '.');
        LogAssertB(strchr(extension, '\\') == nullptr);

        // TODO: Consider using a library function for path building.
        size_t pathLength = strlen(path);
        m_leftSide.reserve(pathLength + 1 + strlen(baseName));

        m_leftSide = path;
        m_leftSide.push_back('\\');
        m_leftSide.append(baseName);

        m_extension = extension;
    }


    std::unique_ptr<std::istream> ParameterizedFile::OpenForRead(const std::string& filename)
    {
        std::ifstream* stream = new std::ifstream(filename.c_str(), std::ifstream::binary);
        stream->exceptions ( std::ifstream::badbit );
        LogAssertB(stream->is_open(), "File %s failed to open for read.", filename.c_str());
        return std::unique_ptr<std::istream>(stream);
    }


    std::string ParameterizedFile::GetTempName(const std::string& filename)
    {
        return filename + ".temp";
    }


    std::unique_ptr<std::ostream> ParameterizedFile::OpenForWrite(const std::string& filename)
    {
        std::ofstream* stream =  new std::ofstream(filename.c_str(), std::ofstream::binary);
        stream->exceptions ( std::ifstream::failbit | std::ifstream::badbit );
        LogAssertB(stream->is_open(), "File %s failed to open for write.", filename.c_str());
        return std::unique_ptr<std::ostream>(stream);
    }


    void ParameterizedFile::Commit(const std::string& filename)
    {
        if (Exists(filename))
        {
            Delete(filename);
        }

        std::string tempFilename = GetTempName(filename);

        BOOL res = MoveFileA(tempFilename.c_str(), filename.c_str());

        LogAssertB(res != 0, "Error %d commit file %s.", GetLastError(), filename.c_str());
    }


    bool ParameterizedFile::Exists(const std::string& filename)
    {
        // DESIGN NOTE: The following stream-based technique will return false
        // in some situations where the file exists. Some examples are when the
        // file exists, but is opened for exclusive access by another process.
        //std::ifstream stream(filename.c_str());
        //bool success = stream.is_open();
        //stream.close();
        //return success;

        return GetFileAttributesA(filename.c_str()) != INVALID_FILE_ATTRIBUTES;
    }


    void ParameterizedFile::Delete(const std::string& filename)
    {
        LogAssertB(DeleteFileA(filename.c_str()) != 0,
                   "Error %d deleting file %s.",
                   GetLastError(),
                   filename.c_str());
    }


    ParameterizedFile0::ParameterizedFile0(const char* path,
                                           const char* baseName,
                                           const char* extension)
        : ParameterizedFile(path, baseName, extension)
    {
    }


    std::string ParameterizedFile0::GetName()
    {
        std::stringstream ss;
        ss << m_leftSide << m_extension;
        return ss.str();
    }


    std::unique_ptr<std::istream> ParameterizedFile0::OpenForRead()
    {
        return ParameterizedFile::OpenForRead(GetName());
    }


    std::unique_ptr<std::ostream> ParameterizedFile0::OpenForWrite()
    {
        return ParameterizedFile::OpenForWrite(GetName());
    }


    std::unique_ptr<std::ostream> ParameterizedFile0::OpenTempForWrite()
    {
        return ParameterizedFile::OpenForWrite(GetTempName(GetName()));
    }


    void ParameterizedFile0::Commit()
    {
        ParameterizedFile::Commit(GetName());
    }


    bool ParameterizedFile0::Exists()
    {
        return ParameterizedFile::Exists(GetName());
    }


    void ParameterizedFile0::Delete()
    {
        ParameterizedFile::Delete(GetName());
    }
}
