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

#include "BitFunnel/Configuration/Factories.h"
#include "RAMFileSystem.h"


namespace BitFunnel
{
    std::unique_ptr<IFileSystem>
        Factories::CreateRAMFileSystem()
    {
        return std::unique_ptr<IFileSystem>(new RAMFileSystem());
    }


    std::unique_ptr<std::ostream>
        RAMFileSystem::OpenForWrite(char const * filename,
                                    std::ios_base::openmode mode)
    {
        auto buffer = EnsureStream(filename, true);
        std::unique_ptr<std::stringstream> stream(new std::stringstream(mode));
        (static_cast<std::ostream*>(stream.get()))->rdbuf(buffer);
        return std::unique_ptr<std::ostream>(stream.release());
    }


    std::unique_ptr<std::istream>
        RAMFileSystem::OpenForRead(char const * filename,
                                   std::ios_base::openmode mode)
    {
        auto buffer = EnsureStream(filename, false);
        std::unique_ptr<std::istream> stream(new std::stringstream(mode));
        stream->rdbuf(buffer);
        stream->seekg(0);
        return stream;
    }


    RAMFileSystem::Buffer
        RAMFileSystem::EnsureStream(const char * filename,
                                    bool forWrite)
    {
        if (forWrite || (m_files.find(filename) == m_files.end()))
        {
            m_files[std::string(filename)] =
                std::unique_ptr<std::stringstream>(new std::stringstream());
            //m_files.insert_or_assign(
            //    std::make_pair(
            //        std::string(filename),
            //        std::unique_ptr<std::stringstream>(new std::stringstream())));
        }

        auto it = m_files.find(filename);
        return it->second->rdbuf();
    }
}
