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


#include <cstring>
#include <fstream>
#include <sys/stat.h>
// #include <Windows.h>                // For DeleteFile.

#include "BitFunnel/Configuration/IFileSystem.h"
#include "LoggerInterfaces/Logging.h"
#include "ParameterizedFile.h"


namespace BitFunnel
{
    ParameterizedFile::ParameterizedFile(IFileSystem & fileSystem,
                                         const char* path,
                                         const char* baseName,
                                         const char* extension)
        : m_fileSystem(fileSystem)
    {
        // TODO: use a multiplatform library for this.
#ifdef BITFUNNEL_PLATFORM_WINDOWS
        // Ensure that the base name does not contain '\' or '.'.
        LogAssertB(strchr(baseName, '\\') == nullptr, "Filename contains \\");
        LogAssertB(strchr(baseName, '.') == nullptr, "Filename contains .");

        // Ensure that extension starts with '.' and does not contain '\'.
        LogAssertB(extension[0] == '.', "Extension doesn't start with .");
        LogAssertB(strchr(extension, '\\') == nullptr, "Extension contains \\");

        // TODO: Consider using a library function for path building.
        size_t pathLength = strlen(path);
        m_leftSide.reserve(pathLength + 1 + strlen(baseName));

        m_leftSide = path;
        m_leftSide.push_back('\\');
        m_leftSide.append(baseName);
#else
        // Ensure that the base name does not contain '\' or '.'.
        LogAssertB(strchr(baseName, '/') == nullptr, "Filename contains /");
        LogAssertB(strchr(baseName, '.') == nullptr, "Filename contains .");

        // Ensure that extension starts with '.' and does not contain '\'.
        LogAssertB(extension[0] == '.', "Extension doesn't start with .");
        LogAssertB(strchr(extension, '/') == nullptr, "Extension contains /");

        // TODO: Consider using a library function for path building.
        size_t pathLength = strlen(path);
        m_leftSide.reserve(pathLength + 1 + strlen(baseName));

        m_leftSide = path;
        m_leftSide.push_back('/');
        m_leftSide.append(baseName);
#endif

        m_extension = extension;
    }


    std::unique_ptr<std::istream> ParameterizedFile::OpenForRead(const std::string& filename)
    {
        return m_fileSystem.OpenForRead(filename.c_str(), std::ios::binary);
    }


    std::string ParameterizedFile::GetTempName(const std::string& filename)
    {
        return filename + ".temp";
    }


    std::unique_ptr<std::ostream> ParameterizedFile::OpenForWrite(const std::string& filename)
    {
        return m_fileSystem.OpenForWrite(filename.c_str(), std::ios::binary);
    }


    // void ParameterizedFile::Commit(const std::string& filename)
    // {
    //     if (Exists(filename))
    //     {
    //         Delete(filename);
    //     }

    //     std::string tempFilename = GetTempName(filename);

    //     BOOL res = MoveFileA(tempFilename.c_str(), filename.c_str());

    //     LogAssertB(res != 0, "Error %d commit file %s.", GetLastError(), filename.c_str());
    // }


     bool ParameterizedFile::Exists(const std::string& filename)
     {
         return m_fileSystem.Exists(filename.c_str());
     }


    // void ParameterizedFile::Delete(const std::string& filename)
    // {
    //     LogAssertB(DeleteFileA(filename.c_str()) != 0,
    //                "Error %d deleting file %s.",
    //                GetLastError(),
    //                filename.c_str());
    // }


    //*************************************************************************
    //
    // ParameterizedFile0
    //
    //*************************************************************************
    ParameterizedFile0::ParameterizedFile0(IFileSystem & fileSystem,
                                           const char* path,
                                           const char* baseName,
                                           const char* extension)
        : ParameterizedFile(fileSystem, path, baseName, extension)
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


    // std::unique_ptr<std::ostream> ParameterizedFile0::OpenTempForWrite()
    // {
    //     return ParameterizedFile::OpenForWrite(GetTempName(GetName()));
    // }


    // void ParameterizedFile0::Commit()
    // {
    //     ParameterizedFile::Commit(GetName());
    // }


     bool ParameterizedFile0::Exists()
     {
         return ParameterizedFile::Exists(GetName());
     }


    // void ParameterizedFile0::Delete()
    // {
    //     ParameterizedFile::Delete(GetName());
    // }




    //*************************************************************************
    //
    // ParameterizedFile1
    //
    //*************************************************************************
     ParameterizedFile1::ParameterizedFile1(IFileSystem & fileSystem,
                                            const char* path,
                                            const char* baseName,
                                            const char* extension)
         : ParameterizedFile(fileSystem, path, baseName, extension)
     {
     }


     std::string ParameterizedFile1::GetName(size_t p1)
     {
         std::stringstream ss;
         ss << m_leftSide << "-" << p1 << m_extension;
         return ss.str();
     }


     std::unique_ptr<std::istream> ParameterizedFile1::OpenForRead(size_t p1)
     {
         return ParameterizedFile::OpenForRead(GetName(p1));
     }


     std::unique_ptr<std::ostream> ParameterizedFile1::OpenForWrite(size_t p1)
     {
         return ParameterizedFile::OpenForWrite(GetName(p1));
     }


     // std::unique_ptr<std::ostream> ParameterizedFile1::OpenTempForWrite(size_t p1)
     // {
     //     return ParameterizedFile::OpenForWrite(GetTempName(GetName(p1)));
     // }


     // void ParameterizedFile1::Commit(size_t p1)
     // {
     //     return ParameterizedFile::Commit(GetName(p1));
     // }


     bool ParameterizedFile1::Exists(size_t p1)
     {
         return ParameterizedFile::Exists(GetName(p1));
     }


     // void ParameterizedFile1::Delete(size_t p1)
     // {
     //     ParameterizedFile::Delete(GetName(p1));
     // }


    //*************************************************************************
    //
    // ParameterizedFile2
    //
    //*************************************************************************
     ParameterizedFile2::ParameterizedFile2(IFileSystem & fileSystem,
                                            const char* path,
                                            const char* baseName,
                                            const char* extension)
         : ParameterizedFile(fileSystem, path, baseName, extension)
     {
     }


     std::string ParameterizedFile2::GetName(size_t p1, size_t p2)
     {
         std::stringstream ss;
         ss << m_leftSide
             << "-" << p1
             << "-" << Converter<size_t>::Convert(p2)
             << m_extension;
         return ss.str();
     }


     std::unique_ptr<std::istream> ParameterizedFile2::OpenForRead(size_t p1, size_t p2)
     {
         return ParameterizedFile::OpenForRead(GetName(p1, p2));
     }


     std::unique_ptr<std::ostream> ParameterizedFile2::OpenForWrite(size_t p1, size_t p2)
     {
         return ParameterizedFile::OpenForWrite(GetName(p1, p2));
     }


     // std::unique_ptr<std::ostream> ParameterizedFile2::OpenTempForWrite(size_t p1, size_t p2)
     // {
     //     return ParameterizedFile::OpenForWrite(GetTempName(GetName(p1, p2)));
     // }


     // void ParameterizedFile2::Commit(size_t p1, size_t p2)
     // {
     //     return ParameterizedFile::Commit(GetName(p1, p2));
     // }


     bool ParameterizedFile2::Exists(size_t p1, size_t p2)
     {
         return ParameterizedFile::Exists(GetName(p1, p2));
     }


     // void ParameterizedFile2::Delete(size_t p1, size_t p2)
     // {
     //     ParameterizedFile::Delete(GetName(p1, p2));
     // }
}
