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

#include <fstream>
#include <sstream>
#include <string>

#include "BitFunnel/IFileManager.h"
// #include "BitFunnel/Tier.h"


namespace BitFunnel
{
    class ParameterizedFile
    {
    public:
        // Constructs a ParameterizedFile based on path, basename,
        // and extension. The file name will equal
        //   path\baseName.extension
        // The path may contain backslashes and may end in a backslash,
        // but this is not required. The baseName cannot contain backslashes or
        // periods. The extension cannot contain periods or backslashes.
        ParameterizedFile(const char* path,
                          const char* baseName,
                          const char* extension);

        std::istream* OpenForRead(const std::string& filename);
    protected:
        std::string GetTempName(const std::string& filename);
        std::ostream* OpenForWrite(const std::string& filename);
        void Commit(const std::string& filename);
        bool Exists(const std::string& filename);
        void Delete(const std::string& filename);

        std::string m_leftSide;
        std::string m_extension;


        template <typename T>
        struct Converter
        {
            static const T& Convert(const T& value)
            {
                return value;
            }
        };


        /* TODO: do we need this?
        template <>
        struct Converter<Tier>
        {
            static const char* Convert(const Tier& value)
            {
                return TierToString(value);
            }
        };
        */
    };


    class ParameterizedFile0 : public IParameterizedFile0, public ParameterizedFile
    {
    public:
        ParameterizedFile0(const char* path,
                           const char* baseName,
                           const char* extension);

        std::string GetName();
        std::istream* OpenForRead();
        std::ostream* OpenForWrite();
        std::ostream* OpenTempForWrite();
        void Commit();
        bool Exists();
        void Delete();
    };


    template <typename P1>
    class ParameterizedFile1 : public IParameterizedFile1<P1>, public ParameterizedFile
    {
    public:
        ParameterizedFile1(const char* path,
                           const char* baseName,
                           const char* extension)
            : ParameterizedFile(path, baseName, extension)
        {
        }


        std::string GetName(P1 p1)
        {
            std::stringstream ss;
            ss << m_leftSide << "-" << p1 << m_extension;
            return ss.str();
        }


        std::istream* OpenForRead(P1 p1)
        {
            return ParameterizedFile::OpenForRead(GetName(p1));
        }


        std::ostream* OpenForWrite(P1 p1)
        {
            return ParameterizedFile::OpenForWrite(GetName(p1));
        }


        std::ostream* OpenTempForWrite(P1 p1)
        {
            return ParameterizedFile::OpenForWrite(GetTempName(GetName(p1)));
        }


        void Commit(P1 p1)
        {
            return ParameterizedFile::Commit(GetName(p1));
        }


        bool Exists(P1 p1)
        {
            return ParameterizedFile::Exists(GetName(p1));
        }


        void Delete(P1 p1)
        {
            ParameterizedFile::Delete(GetName(p1));
        }
    };


    template <typename P1, typename P2>
    class ParameterizedFile2 : public IParameterizedFile2<P1, P2>, public ParameterizedFile
    {
    public:
        ParameterizedFile2(const char* path,
                           const char* baseName,
                           const char* extension)
            : ParameterizedFile(path, baseName, extension)
        {
        }


        std::string GetName(P1 p1, P2 p2)
        {
            std::stringstream ss;
            ss << m_leftSide << "-" << p1 << "-" << Converter<P2>::Convert(p2) << m_extension;
            return ss.str();
        }


        std::istream* OpenForRead(P1 p1, P2 p2)
        {
            return ParameterizedFile::OpenForRead(GetName(p1, p2));
        }


        std::ostream* OpenForWrite(P1 p1, P2 p2)
        {
            return ParameterizedFile::OpenForWrite(GetName(p1, p2));
        }


        std::ostream* OpenTempForWrite(P1 p1, P2 p2)
        {
            return ParameterizedFile::OpenForWrite(GetTempName(GetName(p1, p2)));
        }


        void Commit(P1 p1, P2 p2)
        {
            return ParameterizedFile::Commit(GetName(p1, p2));
        }


        bool Exists(P1 p1, P2 p2)
        {
            return ParameterizedFile::Exists(GetName(p1, p2));
        }


        void Delete(P1 p1, P2 p2)
        {
            ParameterizedFile::Delete(GetName(p1, p2));
        }
    };
}
