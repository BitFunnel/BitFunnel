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

#include <istream>      // std::istream used as a parameter.
#include <ostream>      // std::ostream used as a parameter.


namespace BitFunnel
{
    class Version;

    //*************************************************************************
    //
    // PackedArray provides a memory efficient packed array of n-bit records
    // where n is at least 1 and less than 57.
    //
    // THREAD SAFETY: PackedArray is not thread safe.
    //
    // TODO: REVIEW: Should PackedArray take a useVirtualAlloc parameter or
    // should it just decide which allocator to use. It seems it should pick
    // the allocator. File formats should probably not save the useVirtualAlloc
    // parameter as the decision whether to use VirtualAlloc may be different
    // across machines and OS versions.
    //
    //*************************************************************************
    class PackedArray
    {
    public:
        // Constructs a PackedArray with 'capacity' entries, each with
        // 'bitsPerEntry' bits. If useVirtualAlloc is true, the underlying buffer
        // will be allocated with VirtualAlloc(). This allows for very large
        // PackedArrays that would cause malloc() to fail. If useVirtualAlloc is
        // false, the underlying buffer will be allocated with new [].
        PackedArray(size_t capacity, unsigned bitsPerEntry, bool useVirtualAlloc);

        // Construct a PackedArray from data that was previously persisted to a
        // stream with the Write() method.
        PackedArray(std::istream& input);

        // Destroys the packed array. Its underlying buffer is released with
        // either VirtualFree() or delete [], depending on the value of
        // m_useVirtualAlloc.
        ~PackedArray();

        // Saves the dimensions and contents of the PackedArray to a stream.
        void Write(std::ostream& output) const;

        // Return the number of slots in the PackedArray.
        size_t GetCapacity() const;

        // Returns the value at the specified position in the array.
        uint64_t Get(size_t index) const;

        // Sets the value at the specified position in the array.
        void Set(size_t index, uint64_t value);

        //
        // Static methods below are provided for other classes that want to use
        // PackedArray functionality, but for whatever reason can't use an
        // instance of PackedArray.
        //

        // Returns the size of the buffer in quad words.
        static size_t GetBufferSize(size_t capacity,
                                    unsigned bitsPerEntry);

        // Allocates the underlying buffer, with space for 'capacity' entries,
        // each with 'bitsPerEntry' entries.
        static uint64_t* AllocateBuffer(size_t capacity,
                                                unsigned bitsPerEntry,
                                                bool useVirtualAlloc);


        // Returns the value at the specified index in a packed array of
        // where each entry has 'bitsPerEntry' bits. The mask should be set
        // to (1 << bitsPerEntry) - 1. Does not perform bounds checking
        // on index.
        static uint64_t Get(size_t index,
                                unsigned bitsPerEntry,
                                uint64_t mask,
                                uint64_t* buffer);

        // Sets the value at the specified index in a packed array of
        // where each entry has 'bitsPerEntry' bits. The mask should be set
        // to (1 << bitsPerEntry) - 1. Does not perform bounds checking
        // on index.
        static void Set(size_t index,
                        unsigned bitsPerEntry,
                        uint64_t mask,
                        uint64_t* buffer,
                        uint64_t value);

        // Returns the maximum number of bits supported in an entry.
        static unsigned GetMaxBitsPerEntry();

    private:
        static const Version c_version;
        static const unsigned c_maxBitsPerEntry = 56;

        bool m_useVirtualAlloc;
        size_t m_capacity;
        unsigned m_bitsPerEntry;
        uint64_t m_mask;

        // Buffer that holds packed array values.
        uint64_t* m_buffer;
    };
}
