#include <cerrno>
#include <cstring>
#include <sstream>
#include <stdexcept>

#include "AlignedBuffer.h"
#include "LoggerInterfaces/Logging.h"

#ifdef BITFUNNEL_PLATFORM_WINDOWS
#include <Windows.h>   // For VirtualAlloc/VirtualFree.
#else
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
        // mmap will give us something page aligned and we assume that alignment
        // is sufficient.
        LogAssertB(alignment <= 4096, "Alignment > 4096.\n");
        // TODO: detect non-4k size?
        m_actualSize = m_requestedSize;
        m_rawBuffer = mmap((caddr_t)0, size,
                           PROT_READ | PROT_WRITE,
                           MAP_ANONYMOUS | MAP_PRIVATE,
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
            munmap(m_rawBuffer, m_actualSize);
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
