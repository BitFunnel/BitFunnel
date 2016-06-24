#include "stdafx.h"

#include <stdexcept>
#include <Windows.h>

#include "AlignedBuffer.h"
#include "LoggerInterfaces/Logging.h"


namespace BitFunnel
{

    AlignedBuffer::AlignedBuffer(size_t size, int alignment)
    {
        m_requestedSize = size;
        size_t padding = 1ULL << alignment;
        m_actualSize = m_requestedSize + padding;

        m_rawBuffer = VirtualAlloc(nullptr, m_actualSize, MEM_COMMIT, PAGE_READWRITE);
        LogAssertB(m_rawBuffer != nullptr, "VirtualAlloc() failed.");

        m_alignedBuffer = (char *)(((size_t)m_rawBuffer + padding -1) & ~(padding -1));
    }

    AlignedBuffer::~AlignedBuffer()
    {
        if (m_rawBuffer != nullptr)
        {
            VirtualFree(m_rawBuffer, 0, MEM_RELEASE);
        }
    }

    void *AlignedBuffer::GetBuffer() const
    {
        return m_alignedBuffer;
    }

    size_t AlignedBuffer::GetSize() const
    {
        return m_requestedSize;
    }
}
