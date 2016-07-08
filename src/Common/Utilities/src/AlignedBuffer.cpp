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


#include <cstring>
#include <stdexcept>

#include "AlignedBuffer.h"
#include "LoggerInterfaces/Logging.h"

#ifdef BITFUNNEL_PLATFORM_WINDOWS
#include <Windows.h>   // For VirtualAlloc/VirtualFree.
#else
#include <cerrno>
#include <sstream>
#include <sys/mman.h>  // For mmap/munmap.
#endif


namespace BitFunnel
{

    AlignedBuffer::AlignedBuffer(size_t size, int alignment)
    {
        m_requestedSize = size;

#ifdef BITFUNNEL_PLATFORM_WINDOWS
        size_t padding = 1ULL << alignment;
        m_actualSize = m_requestedSize + padding;
        m_rawBuffer = VirtualAlloc(nullptr, m_actualSize, MEM_COMMIT, PAGE_READWRITE);
        LogAssertB(m_rawBuffer != nullptr, "VirtualAlloc() failed.");
        m_alignedBuffer = (char *)(((size_t)m_rawBuffer + padding -1) & ~(padding -1));
#else
        // TODO: detect non-4k size?
        const int c_pageSize = 4096;

        // mmap will give us something page aligned and we assume that alignment
        // is sufficient.
        LogAssertB(alignment <= c_pageSize, "Alignment > 4096.\n");
        m_actualSize = m_requestedSize;
        m_rawBuffer = mmap(nullptr, size,
                           PROT_READ | PROT_WRITE,
                           MAP_ANON | MAP_PRIVATE,
                           -1,  // No file descriptor.
                           0);
        if (m_rawBuffer == MAP_FAILED)
        {
            std::stringstream errorMessage;
            errorMessage << "AlignedBuffer Failed to mmap: " <<
                std::strerror(errno) <<
                std::endl;
            // TODO: replace this with BitFunnel specific error when that gets
            // committed.
            throw std::runtime_error(errorMessage.str());
        }
        m_alignedBuffer = m_rawBuffer;
#endif
    }

    AlignedBuffer::~AlignedBuffer()
    {
        if (m_rawBuffer != nullptr)
        {
#ifdef BITFUNNEL_PLATFORM_WINDOWS
            VirtualFree(m_rawBuffer, 0, MEM_RELEASE);
#else
            if (munmap(m_rawBuffer, m_actualSize) == -1)
            {
                std::stringstream errorMessage;
                errorMessage << "AlignedBuffer Failed to mmap: " <<
                    std::strerror(errno) <<
                    std::endl;
                // TODO: replace this with BitFunnel specific error when that gets
                // committed.
                throw std::runtime_error(errorMessage.str());
            }
#endif
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
