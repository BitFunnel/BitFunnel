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


#include <istream>

#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/Utilities/ReadLines.h"

namespace BitFunnel
{
    class IFileSystem;

    std::vector<std::string> ReadLines(IFileSystem& fileSystem,
                                       char const * fileName)
    {
        auto input = fileSystem.OpenForRead(fileName, std::ios::in);

        std::vector<std::string> lines;
        std::string line;
        while (std::getline(*input, line)) {
            // Trim whitespace from right side. If we're reading in a file from
            // Windows BitFunnel, we can have a stray \r character.
            line.erase(std::find_if(line.rbegin(),
                                    line.rend(),
                                    std::not1(std::ptr_fun<int, int>(std::isspace))).base(),
                                    line.end());
            // Trim leading whitespace.
            line.erase(line.begin(),
                       std::find_if(line.begin(),
                                    line.end(),
                                    std::not1(std::ptr_fun<int, int>(std::isspace))));
            lines.push_back(std::move(line));
        }

        return lines;
    }
}
