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

#include <algorithm>    // For std::min() or std::max()

#include "BitFunnel/Allocators/IAllocator.h"
#include "BitFunnel/Utilities/IObjectFormatter.h"
#include "BitFunnel/Utilities/IObjectParser.h"
#include "LoggerInterfaces/Logging.h"
#include "StringVector.h"


namespace BitFunnel
{
    StringVector::StringVector(IAllocator& allocator, unsigned initialCapacity)
        : m_allocator(allocator),
          m_capacity(initialCapacity),
          m_size(0),
          m_strings(reinterpret_cast<const char**>(allocator.Allocate(sizeof(const char*) * initialCapacity)))
    {
    }


    StringVector::StringVector(IObjectParser& parser, unsigned initialCapacity)
        : m_allocator(parser.GetAllocator()),
          m_capacity(initialCapacity),
          m_size(0),
          m_strings(reinterpret_cast<const char**>(parser.GetAllocator().Allocate(sizeof(const char*) * initialCapacity)))
    {
        parser.OpenList();
        while (parser.OpenListItem())
        {
            AddString(parser.ParseStringLiteral());
        }
        parser.CloseList();
    }


    void StringVector::Format(IObjectFormatter& formatter) const
    {
        formatter.OpenList();
        for (unsigned i = 0 ; i < m_size; ++i)
        {
            formatter.OpenListItem();
            formatter.FormatStringLiteral(m_strings[i]);
        }
        formatter.CloseList();
    }


    void StringVector::AddString(const char* string)
    {
        if (m_size == m_capacity)
        {
            unsigned newCapacity = (std::max)(2 * m_capacity, m_capacity + c_minimumGrowthQuanta);
            const char** newStrings = reinterpret_cast<const char**>(m_allocator.Allocate(sizeof(const char*) * newCapacity));
            memcpy(newStrings, m_strings, sizeof(const char*) * m_size);

            // WARNING: Do not free or delete [] m_terms since it was allocated
            // from an arena.

            m_strings = newStrings;
            m_capacity = newCapacity;
        }

        m_strings[m_size++] = string;
    }


    unsigned StringVector::GetCapacity() const
    {
        return m_capacity;
    }


    unsigned StringVector::GetSize() const
    {
        return m_size;
    }


    const char* StringVector::operator[](unsigned index) const
    {
        LogAssertB(index < m_size, "index out of range");
        return m_strings[index];
    }


    StringVector const & StringVector::Parse(IObjectParser& parser, unsigned initialCapacity)
    {
        return *new (parser.GetAllocator().Allocate(sizeof(StringVector)))
                    StringVector(parser, initialCapacity);
    }
}
