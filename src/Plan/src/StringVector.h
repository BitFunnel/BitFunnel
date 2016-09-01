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

#include "BitFunnel/NonCopyable.h"


namespace BitFunnel
{
    class IAllocator;
    class IObjectFormatter;
    class IObjectParser;


    class StringVector : public NonCopyable
    {
    public:
        StringVector(IAllocator& allocator, unsigned initialCapacity);
        StringVector(IObjectParser& parser, unsigned initialCapacity);

        void Format(IObjectFormatter& formatter) const;

        void AddString(char const * string);

        unsigned GetCapacity() const;
        unsigned GetSize() const;
        char const * operator[](unsigned index) const;

        static StringVector const & Parse(IObjectParser& parser, unsigned initialCapacity);

    private:
        IAllocator& m_allocator;
        unsigned m_capacity;
        unsigned m_size;
        char const ** m_strings;

        static const unsigned c_minimumGrowthQuanta = 10;
    };
}
