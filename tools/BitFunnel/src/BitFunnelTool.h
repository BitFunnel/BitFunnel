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

#include <vector>                   // std::vector return value.

#include "BitFunnel/IExecutable.h"  // Base class.


namespace BitFunnel
{
    class IFileSystem;

    class BitFunnelTool : public IExecutable
    {
    public:
        BitFunnelTool(IFileSystem& fileSystem);

        //
        // IExecutable methods
        //
        virtual int Main(std::istream& input,
                         std::ostream& output,
                         int argc,
                         char const *argv[]) override;

    private:
        // Constructs the IExecutable associated with the specified subcommand.
        // Returns a nullptr if there is no matching IExecutable.
        std::unique_ptr<IExecutable>
            CreateExecutable(char const * subcommand) const;

        // Returns a vector of C strings, starting with the string in the name
        // parameter, followed by strings from argv[2], arg3[3], etc. The
        // purpose of this method is to convert the BitFunnel argument list
        // into the sub-command argument list, by stripping out the subcommand
        // name.
        std::vector<char const *> FilterArgs(
            int argc,
            char const *argv[],
            char const * name) const;

        // Prints the top-level usage message for the BitFunnel command.
        static void Usage(std::ostream& output);

        IFileSystem& m_fileSystem;
    };
}
