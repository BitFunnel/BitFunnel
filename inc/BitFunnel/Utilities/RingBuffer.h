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

#include "LoggerInterfaces/Logging.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // RingBuffer is a simple queue embedded in a fixed-length circular buffer.
    //
    // The length of the circular buffer is constrained to be a power of two in
    // order to avoid using the mod operator when computing index values.
    //
    // The capacity of the RingBuffer is one less than the size of its circular
    // buffer.
    //
    //*************************************************************************
    template <typename T, size_t LOG2_CAPACITY>
    class RingBuffer
    {
    public:
        // Create an empty RingBuffer.
        RingBuffer();

        // Resets the RingBuffer to an empty state. Note that Reset() will
        // never invoke the destructor for T.
        void Reset();

        // Extends the back of the queue and returns the pointer to the last
        // item. Caller may use this pointer to set value of the last item.
        // This method will assert if the RingBuffer is already full and
        // cannot be extended.
        //
        // DESIGN NOTE: This method returns a T* instead of a T& in order to
        // facilitate a usage pattern where the caller does a placement new of
        // T at the address returned by PushBack. Constructing T in place
        // allows the caller to avoid a copy of T. Once consequence of this
        // design is that RingBuffer doesn't really manage a buffer of T. It
        // never calls T's constructor and destructor.
        T* PushBack();


        // Removed the front item from the queue. This method will assert if
        // the RingBuffer is already empty. Note that this method will not call
        // T::~T().
        void PopFront();

        // Returns a reference to the item in the buffer at the specified index
        // relative to the head item. Asserts if index references a slot beyond
        // the end of the buffer.
        T& operator[](size_t index) const;

        // Returns true if the RingBuffer is empty. In this implementation, the
        // buffer is considered empty when m_head == m_tail.
        bool IsEmpty() const;

        // Returns true if the RingBuffer is full. In this implementation, the
        // buffer is considered full when (m_tail + 1) % c_slotCount == m_head.
        bool IsFull() const;

        // Returns the number of items currently in the buffer.
        size_t GetCount() const;

    private:
        static const size_t c_slotCount = 1 << LOG2_CAPACITY;
        static const size_t c_slotMask = c_slotCount - 1;


        __declspec(align(8)) char m_buffer[sizeof(T) * c_slotCount];

        T* m_slots;
        size_t m_head;
        size_t m_tail;
    };


    template <typename T, size_t LOG2_CAPACITY>
    RingBuffer<T, LOG2_CAPACITY>::RingBuffer()
        : m_slots(reinterpret_cast<T*>(m_buffer)),
          m_head(0),
          m_tail(0)
    {
        static_assert(LOG2_CAPACITY < 32, "RingBuffer: LOG2_CAPACITY must be less than 32.");
    }


    template <typename T, size_t LOG2_CAPACITY>
    void RingBuffer<T, LOG2_CAPACITY>::Reset()
    {
        m_head = 0;
        m_tail = 0;
    }


    template <typename T, size_t LOG2_CAPACITY>
    T* RingBuffer<T, LOG2_CAPACITY>::PushBack()
    {
        LogAssertB(!IsFull());

        T* slot = m_slots + m_tail;
        m_tail = (m_tail + 1) & c_slotMask;

        return slot;
    }


    template <typename T, size_t LOG2_CAPACITY>
    void RingBuffer<T, LOG2_CAPACITY>::PopFront()
    {
        LogAssertB(!IsEmpty());

        m_head = (m_head + 1) & c_slotMask;
    }


    template <typename T, size_t LOG2_CAPACITY>
    T& RingBuffer<T, LOG2_CAPACITY>::operator[](size_t index) const
    {
        size_t slot = m_head + index;
        size_t tail = (m_tail < m_head) ? m_tail + c_slotCount : m_tail;
        LogAssertB(slot < tail);

        return m_slots[slot & c_slotMask];
    }


    template <typename T, size_t LOG2_CAPACITY>
    bool RingBuffer<T, LOG2_CAPACITY>::IsEmpty() const
    {
        return m_head == m_tail;
    }


    template <typename T, size_t LOG2_CAPACITY>
    bool RingBuffer<T, LOG2_CAPACITY>::IsFull() const
    {
        return ((m_tail + 1) & c_slotMask) == m_head;
    }


    template <typename T, size_t LOG2_CAPACITY>
    size_t RingBuffer<T, LOG2_CAPACITY>::GetCount() const
    {
        if (m_head <= m_tail)
        {
            return m_tail - m_head;
        }
        else
        {
            return m_tail + c_slotCount - m_head;
        }
    }
}
