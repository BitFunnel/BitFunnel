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

#include <iterator>
#include <memory>       // std::unique_ptr
#include <type_traits>
#include <stddef.h>     // size_t embedded.
#include <vector>       // Inline method.

#include "BitFunnel/Index/Factories.h"      // TODO: Remove this include after remving inline.


namespace BitFunnel
{
    class Slice;

    class ResultsBuffer
    {
    public:
		class const_iterator;

        class Result
        {
        public:
            DocumentHandle GetHandle() const
            {
                return Factories::CreateDocumentHandle(m_slice, m_index);
            }

            bool operator<(Result const & other) const
            {
                if (m_slice != other.m_slice)
                {
                    return m_index < other.m_index;
                }
                else
                {
                    return m_slice < other.m_slice;
                }
            }

            Slice* m_slice;
            size_t m_index;
        };
        static_assert(std::is_standard_layout<Result>::value,
                      "Generated code requires standard layout for Result.");
        static_assert(std::is_trivially_copyable<Result>::value,
                      "Generated code requires that Result be trivially copyable.");

        ResultsBuffer(size_t capacity)
          : m_bufferOwner(new Result[capacity]),
            m_capacity(capacity),
            m_size(0)
        {
            m_buffer = m_bufferOwner.get();
        }

        void Reset()
        {
            m_size = 0;
        }

        void push_back(Slice* slice, size_t index)
        {
            // TODO: add overflow check
            m_buffer[m_size].m_slice = slice;
            m_buffer[m_size].m_index = index;
            m_size++;
        }

        const_iterator begin() const
        {
            return const_iterator(m_buffer);
        }

        const_iterator end() const
        {
            return const_iterator(m_buffer + m_size);
        }

        size_t size() const
        {
            return m_size;
        }

        std::unique_ptr<Result[]> m_bufferOwner;
        size_t m_capacity;
        size_t m_size;
        Result * m_buffer;

		class const_iterator
			: public std::iterator<std::input_iterator_tag, Result>
		{
		public:
			const_iterator(Result * result)
				: m_current(result)
			{
			}

			bool operator!=(const_iterator const & other) const
			{
				return m_current != other.m_current;
			}

			const_iterator& operator++()
			{
				m_current++;
				return *this;
			}

			Result const operator*() const
			{
				return *m_current;
			}

		private:
			Result const * m_current;
		};

	};
    static_assert(std::is_standard_layout<ResultsBuffer>::value,
                  "Generated code requires standard layout for ResultsBuffer.");
}
