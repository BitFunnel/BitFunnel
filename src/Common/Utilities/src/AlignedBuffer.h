#pragma once


namespace BitFunnel
{
    //*************************************************************************
    //
    // AlignedBuffer provides a block of memory that is aligned to a 2^alignment
    // boundary. This is intended to be used for allocating "large" blocks of
    // memory, something like 10GB or 100GB at a time.
    //
    //*************************************************************************
    class AlignedBuffer
    {
    public:
        AlignedBuffer(size_t size, int alignment);
        ~AlignedBuffer();

        void *GetBuffer() const;
        size_t GetSize() const;

    private:
        size_t m_requestedSize;
        size_t m_actualSize;
        void *m_rawBuffer;
        void *m_alignedBuffer;
    };
}
