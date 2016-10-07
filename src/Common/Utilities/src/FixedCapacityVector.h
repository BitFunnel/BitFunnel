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
    // FixedCapacityVector<T, CAPACITY> is a variable-sized array of T,
    // optimized for immutable types and non-POD types where assignmentt
    // semantics are no available or expensive. Items are added to the array
    // by constructing them in place using the overload of the PushBack()
    // method whose parameters are the same as T::T().
    //
    // FixedCapacityArrays are variable-sized up to the capacity limit
    // specified at construction time. The do not reallocate like std::vector.
    //
    // The destructor for FixedCapacityVector invokes T::~T() for each item in
    // the array.
    //
    //*************************************************************************
    template <class T, unsigned CAPACITY>
    class FixedCapacityVector
    {
    public:
        // Construct an empty FixedCapacityVector.
        FixedCapacityVector();

        // Invokes T::~T() on every item in the array.
        ~FixedCapacityVector();

        // Appends a new T to the end of the array. Invokes item constructor
        // T::T().
        T& PushBack();

        // Appends a new T to the end of the array. Invokes item constructor
        // T::T(a).
        template <typename A>
        T& PushBack(const A& a);

        // Appends a new T to the end of the array. Invokes item constructor
        // T::T(a, b).
        template <typename A, typename B>
        T& PushBack(const A& a, const B& b);

        // Appends a new T to the end of the array. Invokes item constructor
        // T::T(a, b, c).
        template <typename A, typename B, typename C>
        T& PushBack(const A& a, const B& b, const C& c);

        // Appends a new T to the end of the array. Invokes item constructor
        // T::T(a, b, c, d).
        template <typename A, typename B, typename C, typename D>
        T& PushBack(const A& a, const B& b, const C& c, const D& d);

        // Returns the number of items currently stored in the array.
        unsigned GetSize() const;

        // Returns the capacity of the array. This is the maximum number of
        // items that can be stored in teh array.
        unsigned GetCapacity() const;

        // Returns the item at the specified index. Asserts if the item is out
        // of range.
        T& operator[](unsigned index);

        // Const version of operator[].
        const T& operator[](unsigned index) const;

        T& Back();

        const T& Back() const;

    protected:
        // Protected members are for access by subclass FixedCapacityPODVector.

        unsigned m_size;
        T* m_entries;

    private:
        char m_buffer[sizeof(T) * CAPACITY];
    };


    //*************************************************************************
    //
    // Template method definitions for FixedCapacityVector<T, CAPACITY>.
    //
    //*************************************************************************
    template <class T, unsigned CAPACITY>
    FixedCapacityVector<T, CAPACITY>::FixedCapacityVector()
        : m_size(0)
    {
        // TODO: What about alignment?
        m_entries = reinterpret_cast<T*>(m_buffer);
    }


    template <class T, unsigned CAPACITY>
    FixedCapacityVector<T, CAPACITY>::~FixedCapacityVector()
    {
        for (unsigned i = 0 ; i < m_size; ++i)
        {
            m_entries[i].~T();
        }
    }


    template <typename T, unsigned CAPACITY>
    T& FixedCapacityVector<T, CAPACITY>::PushBack()
    {
        LogAssertB(m_size < CAPACITY, "FixedCapacityVector overflow.");
        T* object = new (m_entries + m_size) T();

        // Only increment m_size if call to T::T() was successful. This
        // is important because ~FixedCapacityVector() invokes T::~T() for all
        // items in the array.
        ++m_size;

        return *object;
    }


    template <typename T, unsigned CAPACITY>
    template <typename A>
    T& FixedCapacityVector<T, CAPACITY>::PushBack(const A& a)
    {
        LogAssertB(m_size < CAPACITY, "FixedCapacityVector overflow.");
        T* object = new (m_entries + m_size) T(a);

        // Only increment m_size if call to T::T() was successful. This
        // is important because ~FixedCapacityVector() invokes T::~T() for all
        // items in the array.
        ++m_size;

        return *object;
    }


    template <typename T, unsigned CAPACITY>
    template <typename A, typename B>
    T& FixedCapacityVector<T, CAPACITY>::PushBack(const A& a, const B& b)
    {
        LogAssertB(m_size < CAPACITY, "FixedCapacityVector overflow.");
        T* object = new (m_entries + m_size) T(a, b);

        // Only increment m_size if call to T::T() was successful. This
        // is important because ~FixedCapacityVector() invokes T::~T() for all
        // items in the array.
        ++m_size;

        return *object;
    }


    template <typename T, unsigned CAPACITY>
    template <typename A, typename B, typename C>
    T& FixedCapacityVector<T, CAPACITY>::PushBack(const A& a, const B& b, const C& c)
    {
        LogAssertB(m_size < CAPACITY, "FixedCapacityVector overflow.");
        T* object = new (m_entries + m_size) T(a, b, c);

        // Only increment m_size if call to T::T() was successful. This
        // is important because ~FixedCapacityVector() invokes T::~T() for all
        // items in the array.
        ++m_size;

        return *object;
    }


    template <typename T, unsigned CAPACITY>
    template <typename A, typename B, typename C, typename D>
    T& FixedCapacityVector<T, CAPACITY>::PushBack(const A& a, const B& b, const C& c, const D& d)
    {
        LogAssertB(m_size < CAPACITY, "FixedCapacityVector overflow.");
        T* object = new (m_entries + m_size) T(a, b, c, d);

        // Only increment m_size if call to T::T() was successful. This
        // is important because ~FixedCapacityVector() invokes T::~T() for all
        // items in the array.
        ++m_size;

        return *object;
    }


    template <class T, unsigned CAPACITY>
    unsigned FixedCapacityVector<T, CAPACITY>::GetCapacity() const
    {
        return CAPACITY;
    }


    template <class T, unsigned CAPACITY>
    unsigned FixedCapacityVector<T, CAPACITY>::GetSize() const
    {
        return m_size;
    }


    template <class T, unsigned CAPACITY>
    T& FixedCapacityVector<T, CAPACITY>::operator[](unsigned index)
    {
        LogAssertB(index < m_size, "out of bounds access.");
        return m_entries[index];
    }


    template <class T, unsigned CAPACITY>
    const T& FixedCapacityVector<T, CAPACITY>::operator[](unsigned index) const
    {
        LogAssertB(index < m_size, "out of bounds access.");
        return m_entries[index];
    }


    template <class T, unsigned CAPACITY>
    T& FixedCapacityVector<T, CAPACITY>::Back()
    {
        LogAssertB(m_size > 0, "underflow.");
        return m_entries[m_size - 1];
    }


    template <class T, unsigned CAPACITY>
    const T& FixedCapacityVector<T, CAPACITY>::Back() const
    {
        LogAssertB(m_size > 0, "underflow.");
        return m_entries[m_size - 1];
    }
}
