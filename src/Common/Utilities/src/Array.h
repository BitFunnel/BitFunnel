#pragma once

#include <istream>              // std::istream used as parameter.
#include <ostream>              // std::ostream used as parameter.

#include "BitFunnel/Utilities/StreamUtilities.h"    // Template code calls.
#include "SimpleBuffer.h"       // Embeds SimpleBuffer.



namespace BitFunnel
{
    //*************************************************************************
    //
    // Array<T> is a runtime-sized one-dimensional array of T. Array uses
    // VirtualAlloc() internally when the underlying array buffer is too large
    // for operator new. This allows Array to be used for multi-gigabyte
    // arrays.
    //
    //*************************************************************************
    template <typename T>
    class Array
    {
    public:
        // Construct an Array with size elements.
        Array(size_t size);

        // Constructs an Array from data previously persisted to a stream by
        // the Write() method.
        Array(std::istream& input);

        // Destructor.
        ~Array();

        // Writes the dimensions and contents of the array to a stream.
        void Write(std::ostream& output);

        // Returns a reference to the element in the row i and column j.
        T& operator[](size_t i);

        // Const vertion of At().
        const T& operator[](size_t i) const;

        // Returns the number of entries in the array.
        size_t GetSize() const;

    private:
        size_t m_size;
        SimpleBuffer m_buffer;
        T* m_entries;
    };


    //*************************************************************************
    //
    // Array2D<T> is a runtime-sized two-dimensional array of T. Array2D uses
    // VirtualAlloc() internally when the underlying array buffer is too large
    // for operator new. This allows Array2D to be used for multi-gigabyte
    // arrays.
    //
    //*************************************************************************
    template <typename T>
    class Array2D
    {
    public:
        // Construct an Array2D with size1 rows and size2 columns.
        Array2D(size_t size1, size_t size2);

        // Constructs an Array2D from data previously persisted to a stream by
        // the Write() method.
        Array2D(std::istream& input);

        // Destructor.
        ~Array2D();

        // Writes the dimensions and contents of the array to a stream.
        void Write(std::ostream& output);

        // Returns a reference to the element in the row i and column j.
        T& At(size_t i, size_t j);

        // Const vertion of At().
        const T& At(size_t i, size_t j) const;

        // Returns the number of rows in the array.
        size_t GetSize1() const;

        // Returns the number of columns in the array.
        size_t GetSize2() const;

    private:
        size_t m_size1;
        size_t m_size2;
        SimpleBuffer m_buffer;
        T* m_entries;
    };


    //*************************************************************************
    //
    // ArrayFixed<T, A> is a compile-time sized one-dimensional array.
    //
    //*************************************************************************
    template <typename T, size_t A>
    class ArrayFixed
    {
    public:
        ArrayFixed();
        ArrayFixed(std::istream& input);

        ~ArrayFixed();

        void Write(std::ostream& output) const;

        T& operator[](unsigned i);

        const T& operator[](unsigned i) const;

        size_t GetSize() const;

    private:
        SimpleBuffer m_buffer;
        T* m_entries;
    };


    //*************************************************************************
    //
    // Array2DFixed<T, A, B> is a compile-time sized two-dimensional array.
    //
    //*************************************************************************
    template <typename T, size_t A, size_t B>
    class Array2DFixed
    {
    public:
        Array2DFixed();
        Array2DFixed(std::istream& input);

        ~Array2DFixed();

        void Write(std::ostream& output) const;

        T& At(unsigned i, unsigned j);

        const T& At(unsigned i, unsigned j) const;

        size_t GetSize1() const;
        size_t GetSize2() const;

    private:
        SimpleBuffer m_buffer;
        T* m_entries;
    };


    //*************************************************************************
    //
    // Array3DFixed<T, A, B, C> is a compile-time sized three-dimensional array.
    //
    //*************************************************************************
    template <typename T, size_t A, size_t B, size_t C>
    class Array3DFixed
    {
    public:
        Array3DFixed();
        Array3DFixed(std::istream& input);

        ~Array3DFixed();

        void Write(std::ostream& output) const;

        T& At(unsigned i, unsigned j, unsigned k);
        size_t GetSize1() const;
        size_t GetSize2() const;
        size_t GetSize3() const;

    private:
        SimpleBuffer m_buffer;
        T* m_entries;
    };


    //*************************************************************************
    //
    // Array<T> method definitions
    //
    //*************************************************************************
    template <typename T>
    Array<T>::Array(size_t size)
        : m_size(size),
          m_buffer(size * sizeof(T))
    {
        m_entries = reinterpret_cast<T*>(m_buffer.GetBuffer());

        // Initialize array.
        for (size_t i = 0; i < m_size; ++i)
        {
            new (m_entries + i) T();
        }
    }


    // TODO: need sizes before buffer can be allocated.
    template <typename T>
    Array<T>::Array(std::istream& input)
        : m_buffer(0)
    {
        // Initialize array.
        m_size = StreamUtilities::ReadField<uint64_t>(input);

        m_buffer.Resize(m_size * sizeof(T));
        m_entries = reinterpret_cast<T*>(m_buffer.GetBuffer());

        StreamUtilities::ReadArray(input, m_entries, m_size);
    }


    template <typename T>
    Array<T>::~Array()
    {
        for (size_t i = 0; i < m_size; ++i)
        {
            m_entries[i].~T();
        }
    }


    template <typename T>
    void Array<T>::Write(std::ostream& output)
    {
        StreamUtilities::WriteField<uint64_t>(output, m_size);
        StreamUtilities::WriteArray(output, m_entries, m_size);
    }


    template <typename T>
    T& Array<T>::operator[](size_t i)
    {
        return m_entries[i];
    }


    template <typename T>
    const T& Array<T>::operator[](size_t i) const
    {
        return m_entries[i];
    }


    template <typename T>
    size_t Array<T>::GetSize() const
    {
        return m_size;
    }


    //*************************************************************************
    //
    // Array2D<T> method definitions
    //
    //*************************************************************************
    template <typename T>
    Array2D<T>::Array2D(size_t size1, size_t size2)
        : m_size1(size1),
          m_size2(size2),
          m_buffer(size1 * size2 * sizeof(T))
    {
        m_entries = reinterpret_cast<T*>(m_buffer.GetBuffer());

        // Initialize array.
        for (size_t i = 0; i < m_size1 * m_size2; ++i)
        {
            new (m_entries + i) T();
        }
    }


    // TODO: need sizes before buffer can be allocated.
    template <typename T>
    Array2D<T>::Array2D(std::istream& input)
        : m_buffer(0)
    {
        // Initialize array.
        m_size1 = StreamUtilities::ReadField<uint64_t>(input);
        m_size2 = StreamUtilities::ReadField<uint64_t>(input);

        m_buffer.Resize(m_size1 * m_size2 * sizeof(T));
        m_entries = reinterpret_cast<T*>(m_buffer.GetBuffer());

        StreamUtilities::ReadArray(input, m_entries, m_size1 * m_size2);
    }


    template <typename T>
    Array2D<T>::~Array2D()
    {
        for (size_t i = 0; i < m_size1 * m_size2; ++i)
        {
            m_entries[i].~T();
        }
    }


    template <typename T>
    void Array2D<T>::Write(std::ostream& output)
    {
        StreamUtilities::WriteField<uint64_t>(output, m_size1);
        StreamUtilities::WriteField<uint64_t>(output, m_size2);
        StreamUtilities::WriteArray(output, m_entries, m_size1 * m_size2);
    }


    template <typename T>
    T& Array2D<T>::At(size_t i, size_t j)
    {
        return m_entries[(i * m_size2) + j];
    }


    template <typename T>
    const T& Array2D<T>::At(size_t i, size_t j) const
    {
        return m_entries[(i * m_size2) + j];
    }


    template <typename T>
    size_t Array2D<T>::GetSize1() const
    {
        return m_size1;
    }


    template <typename T>
    size_t Array2D<T>::GetSize2() const
    {
        return m_size2;
    }


    //*************************************************************************
    //
    // ArrayFixed<T, A> method definitions
    //
    //*************************************************************************
    template <typename T, size_t A>
    ArrayFixed<T, A>::ArrayFixed()
        : m_buffer(A * sizeof(T))
    {
        m_entries = reinterpret_cast<T*>(m_buffer.GetBuffer());

        // Initialize array.
        for (size_t i = 0; i < A; ++i)
        {
            new (m_entries + i) T();
        }
    }


    template <typename T, size_t A>
    ArrayFixed<T, A>::ArrayFixed(std::istream& input)
        : m_buffer(A * sizeof(T))
    {
        size_t a = StreamUtilities::ReadField<uint64_t>(input);
        LogAssertB(A == a, "Read incorrect number of bytes.");

        m_entries = reinterpret_cast<T*>(m_buffer.GetBuffer());
        StreamUtilities::ReadArray(input, m_entries, A);
    }


    template <typename T, size_t A>
    ArrayFixed<T, A>::~ArrayFixed()
    {
        for (size_t i = 0; i < A; ++i)
        {
            m_entries[i].~T();
        }
    }


    template <typename T, size_t A>
    void ArrayFixed<T, A>::Write(std::ostream& output) const
    {
        StreamUtilities::WriteField<uint64_t>(output, A);

        StreamUtilities::WriteArray(output, m_entries, A);
    }


    template <typename T, size_t A>
    T& ArrayFixed<T, A>::operator[](unsigned i)
    {
        return m_entries[i];
    }


    template <typename T, size_t A>
    const T& ArrayFixed<T, A>::operator[](unsigned i) const
    {
        return m_entries[i];
    }


    template <typename T, size_t A>
    size_t ArrayFixed<T, A>::GetSize() const
    {
        return A;
    }


    //*************************************************************************
    //
    // Array2DFixed<T, A, B> method definitions
    //
    //*************************************************************************
    template <typename T, size_t A, size_t B>
    Array2DFixed<T, A, B>::Array2DFixed()
        : m_buffer(A * B * sizeof(T))
    {
        m_entries = reinterpret_cast<T*>(m_buffer.GetBuffer());

        // Initialize array.
        for (size_t i = 0; i < A * B; ++i)
        {
            new (m_entries + i) T();
        }
    }


    template <typename T, size_t A, size_t B>
    Array2DFixed<T, A, B>::Array2DFixed(std::istream& input)
        : m_buffer(A * B * sizeof(T))
    {
        size_t a = StreamUtilities::ReadField<uint64_t>(input);
        LogAssertB(A == a, "Read incorrect number of bytes.");

        size_t b = StreamUtilities::ReadField<uint64_t>(input);
        LogAssertB(B == b, "Read incorrect number of bytes.");

        m_entries = reinterpret_cast<T*>(m_buffer.GetBuffer());
        StreamUtilities::ReadArray(input, m_entries, A * B);
    }


    template <typename T, size_t A, size_t B>
    Array2DFixed<T, A, B>::~Array2DFixed()
    {
        for (size_t i = 0; i < A * B; ++i)
        {
            m_entries[i].~T();
        }
    }


    template <typename T, size_t A, size_t B>
    void Array2DFixed<T, A, B>::Write(std::ostream& output) const
    {
        StreamUtilities::WriteField<uint64_t>(output, A);
        StreamUtilities::WriteField<uint64_t>(output, B);

        StreamUtilities::WriteArray(output, m_entries, A * B);
    }


    template <typename T, size_t A, size_t B>
    T& Array2DFixed<T, A, B>::At(unsigned i, unsigned j)
    {
        return m_entries[(i * B) + j];
    }


    template <typename T, size_t A, size_t B>
    const T& Array2DFixed<T, A, B>::At(unsigned i, unsigned j) const
    {
        return m_entries[(i * B) + j];
    }


    template <typename T, size_t A, size_t B>
    size_t Array2DFixed<T, A, B>::GetSize1() const
    {
        return A;
    }


    template <typename T, size_t A, size_t B>
    size_t Array2DFixed<T, A, B>::GetSize2() const
    {
        return B;
    }


    //*************************************************************************
    //
    // Array3DFixed<T, A, B, C> method definitions
    //
    //*************************************************************************
    template <typename T, size_t A, size_t B, size_t C>
    Array3DFixed<T, A, B, C>::Array3DFixed()
        : m_buffer(A * B * C * sizeof(T))
    {
        m_entries = reinterpret_cast<T*>(m_buffer.GetBuffer());

        // Initialize array.
        for (size_t i = 0; i < A * B * C; ++i)
        {
            new (m_entries + i) T();
        }
    }


    template <typename T, size_t A, size_t B, size_t C>
    Array3DFixed<T, A, B, C>::Array3DFixed(std::istream& input)
        : m_buffer(A * B * C * sizeof(T))
    {
        size_t a = StreamUtilities::ReadField<uint64_t>(input);
        LogAssertB(A == a, "Read incorrect number of bytes.");

        size_t b = StreamUtilities::ReadField<uint64_t>(input);
        LogAssertB(B == b, "Read incorrect number of bytes.");

        size_t c = StreamUtilities::ReadField<uint64_t>(input);
        LogAssertB(C == c, "Read incorrect number of bytes.");

        m_entries = reinterpret_cast<T*>(m_buffer.GetBuffer());
        StreamUtilities::ReadArray(input, m_entries, A * B * C);
    }


    template <typename T, size_t A, size_t B, size_t C>
    Array3DFixed<T, A, B, C>::~Array3DFixed()
    {
        for (size_t i = 0; i < A * B * C; ++i)
        {
            m_entries[i].~T();
        }
    }


    template <typename T, size_t A, size_t B, size_t C>
    void Array3DFixed<T, A, B, C>::Write(std::ostream& output) const
    {
        StreamUtilities::WriteField<uint64_t>(output, A);
        StreamUtilities::WriteField<uint64_t>(output, B);
        StreamUtilities::WriteField<uint64_t>(output, C);

        StreamUtilities::WriteArray(output, m_entries, A * B * C);
    }


    template <typename T, size_t A, size_t B, size_t C>
    T& Array3DFixed<T, A, B, C>::At(unsigned i, unsigned j, unsigned k)
    {
        return m_entries[((i * B + j) * C) + k];
    }


    template <typename T, size_t A, size_t B, size_t C>
    size_t Array3DFixed<T, A, B, C>::GetSize1() const
    {
        return A;
    }


    template <typename T, size_t A, size_t B, size_t C>
    size_t Array3DFixed<T, A, B, C>::GetSize2() const
    {
        return B;
    }


    template <typename T, size_t A, size_t B, size_t C>
    size_t Array3DFixed<T, A, B, C>::GetSize3() const
    {
        return C;
    }
}
