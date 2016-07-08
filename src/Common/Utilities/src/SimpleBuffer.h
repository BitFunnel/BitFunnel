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
