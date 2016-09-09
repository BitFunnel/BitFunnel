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

#include <cstring>                              // Call to memcpy.
#include <string>                               // std::string parameter.

#include "BitFunnel/Allocators/IAllocator.h"
#include "BitFunnel/Exceptions.h"
#include "BitFunnel/NonCopyable.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // StringBuilder
    //
    // A class that assists in constructing zero-terminated C strings from
    // arena-allocated memory.
    //
    //*************************************************************************
    class StringBuilder : public NonCopyable
    {
    public:
        static const size_t c_initialCapacity = 32;

        // Constructs a StringBuilder that will allocate its string buffer
        // from a specified IAllocator. The initial buffer will have space for
        // at least 'capacity' characters, but may be larger.
        StringBuilder(IAllocator& allocator,
                      size_t capacity = c_initialCapacity)
            : m_allocator(allocator)
        {
            // For this constructor, use requested capacity.
            Initialize(capacity);
        }


        // Constructs a StringBuilder that will allocate its string buffer
        // from a specified IAllocator.
        StringBuilder(IAllocator& allocator,
                      char const * s)
            : m_allocator(allocator)
        {
            // Initialize with an additional c_initialCapacity so that
            // the next call to push_char() or append() is less likely
            // to extend.
            Initialize(strlen(s) + c_initialCapacity);
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4996)
#endif
            strcpy(m_buffer, s);
#ifdef _MSC_VER
#pragma warning(pop)
#endif
        }


        // Constructs a StringBuilder that will allocate its string buffer
        // from a specified IAllocator.
        StringBuilder(IAllocator& allocator,
                      std::string const & s)
            : m_allocator(allocator)
        {
            // Initialize with an additional c_initialCapacity so that
            // the next call to push_char() or append() is less likely
            // to extend.
            Initialize(s.size() + c_initialCapacity);
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4996)
#endif
            strcpy(m_buffer, s.c_str());
#ifdef _MSC_VER
#pragma warning(pop)
#endif
        }


        // Extends the string under construction by a single character.
        void push_back(char c)
        {
            EnsureSpace(1);

            m_buffer[m_size++] = c;
            m_buffer[m_size] = 0;
        }


        // Appends a zero-terminated C string to the string under construction.
        void append(char const * s)
        {
            size_t count = strlen(s);
            EnsureSpace(count);
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4996)
#endif
            strcpy(m_buffer + m_size, s);
#ifdef _MSC_VER
#pragma warning(pop)
#endif
            m_size += count;
        }


        // Appends an std::string to the string under construction.
        void append(std::string const & s)
        {
            EnsureSpace(s.size());
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4996)
#endif
            strcpy(m_buffer + m_size, s.c_str());
#ifdef _MSC_VER
#pragma warning(pop)
#endif
            m_size += s.size();
        }


        // Returns the string under construction.
        operator char*() const
        {
            return m_buffer;
        }


    private:
        void Initialize(size_t capacity)
        {
            // Handling zero capacity adds too much complexity, so ensure
            // capacity is at least 1.
            if (m_capacity == 0)
            {
                m_capacity = 1;
            }

            m_capacity = capacity;
            m_size = 0;
            m_buffer = static_cast<char*>(m_allocator.Allocate(capacity + 1));

            // m_buffer holds a zero-terminated C string. Add trailing zero.
            *m_buffer = 0;
        }


        void EnsureSpace(size_t count)
        {
            size_t newSize = m_size + count;
            size_t newCapacity = m_capacity;

            while (newCapacity < newSize)
            {
                if (newCapacity == 0)
                {
                    newCapacity = 1;
                }
                else
                {
                    newCapacity *= 2;
                }
            }

            m_capacity = newCapacity;
            char * newBuffer =
                static_cast<char*>(m_allocator.Allocate(newCapacity + 1));

            memcpy(newBuffer, m_buffer, m_size + 1);

            // NOTE: ok to walk away from original m_buffer because it was
            // was allocated from an arena (IAllocator).
            m_buffer = newBuffer;
        }

        IAllocator & m_allocator;
        size_t m_capacity;
        size_t m_size;
        char* m_buffer;
    };
}
