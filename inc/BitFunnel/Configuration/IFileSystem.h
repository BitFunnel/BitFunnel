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

#include <iosfwd>                   // std::istream, std::ostream return values.
#include <memory>                   // std::unique_ptr return value.

#include "BitFunnel/IInterface.h"   // Base class.

#ifdef __clang__
// Pure abstract classes "should" have a vtable in every translation unit.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wweak-vtables"
#endif

namespace BitFunnel
{
    class IFileSystem : public IInterface
    {
    public:
        virtual std::unique_ptr<std::ostream>
            OpenForWrite(char const * filename,
                         std::ios_base::openmode mode = std::ios::out) = 0;
        virtual std::unique_ptr<std::istream>
            OpenForRead(char const * filename,
                        std::ios_base::openmode mode = std::ios::in) = 0;

        virtual bool Exists(char const * filename) = 0;
    };
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
