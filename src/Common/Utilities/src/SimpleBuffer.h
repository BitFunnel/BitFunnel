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

namespace BitFunnel
{
    //*************************************************************************
    //
    // SimpleBuffer controls allocation and deallocation of a buffer of char in
    // order to facilitate development of RAII classes the require buffers.
    //
    // Internally SimpleBuffer uses VirtualAlloc if the specified capacity is
    // too large for operator new.
    //
    //*************************************************************************
    class SimpleBuffer
    {
    public:
        // Constructs a SimpleBuffer with a specified capacity.
        SimpleBuffer(size_t capacity = 0);

        // Destroys a SimpleBuffer.
        ~SimpleBuffer();

        // Releases the original buffer and then allocates a new one with the
        // specified capacity. WARNING: Does not copy the contents of the old
        // buffer to the new buffer.
        void Resize(size_t capacity);

        // Returns a pointer to the buffer.
        char* GetBuffer() const;

    private:
        static const size_t c_virtualAllocThreshold = 1UL << 30;

        void AllocateBuffer(size_t capacity);
        void FreeBuffer();

        bool m_usedVirtualAlloc;
        char* m_buffer;
        size_t m_capacity;
    };
}
