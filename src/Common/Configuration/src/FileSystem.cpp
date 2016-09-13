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

#include "BitFunnel/Exceptions.h"
#include "FileSystem.h"


namespace BitFunnel
{
    std::unique_ptr<std::ostream>
        FileSystem::OpenForWrite(char const * filename)
    {
        std::unique_ptr<std::ofstream> stream(
            new std::ofstream(filename, std::ofstream::binary));

        stream->exceptions(std::ofstream::badbit |
                           std::ofstream::badbit);

        if (!stream->is_open())
        {
            std::stringstream message;
            message
                << "File "
                << filename
                << " failed to open for write.";
            RecoverableError error(message.str().c_str());
            throw error;
        }

        return std::unique_ptr<std::ostream>(stream.release());
    }


    std::unique_ptr<std::istream>
        FileSystem::OpenForRead(char const * filename)
    {
        std::unique_ptr<std::ifstream> stream(
            new std::ifstream(filename, std::ifstream::binary));

        stream->exceptions(std::ifstream::badbit);

        if (!stream->is_open())
        {
            std::stringstream message;
            message
                << "File "
                << filename
                << " failed to open for read.";
            RecoverableError error(message.str().c_str());
            throw error;
        }

        return std::unique_ptr<std::istream>(stream.release());
    }
}
