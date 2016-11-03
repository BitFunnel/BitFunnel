#ifdef BITFUNNEL_PLATFORM_WINDOWS
#include <Windows.h>   // For VirtualAlloc/VirtualFree.
#else
#include <cerrno>
#include <cstring>
#include <sstream>
#include <sys/mman.h>  // For mmap/munmap.
#endif

#include "BitFunnel/Exceptions.h"
#include "LoggerInterfaces/Logging.h"
#include "SimpleBuffer.h"


namespace BitFunnel
{
    SimpleBuffer::SimpleBuffer(size_t capacity)
        : m_buffer(nullptr),    // nullptr indicates buffer has not yet been allocated.
          m_capacity(0)
    {
        AllocateBuffer(capacity);
    }


    SimpleBuffer::~SimpleBuffer()
    {
        FreeBuffer();
    }


    void SimpleBuffer::Resize(size_t capacity)
    {
        FreeBuffer();
        AllocateBuffer(capacity);
    }


    char* SimpleBuffer::GetBuffer() const
    {
        return m_buffer;
    }


    // DESIGN NOTE: m_capacity is only necessary when using mmap, but we also
    // set the value when we use VirtualAlloc for consistency.
    void SimpleBuffer::AllocateBuffer(size_t capacity)
    {
        LogAssertB(m_buffer == nullptr, "double Allocate.");
        // TODO: do we need different thresholds for different platforms?
        if (capacity >= c_virtualAllocThreshold)
        {
            m_usedVirtualAlloc = true;
#ifdef BITFUNNEL_PLATFORM_WINDOWS
            m_buffer = static_cast<char*>(VirtualAlloc(nullptr, capacity, MEM_COMMIT, PAGE_READWRITE));
            LogAssertB(m_buffer != nullptr, "VirtualAlloc() failed.");
#else
            m_buffer = static_cast<char*>(mmap(nullptr, capacity,
                                               PROT_READ | PROT_WRITE,
                                               MAP_ANON | MAP_PRIVATE,
                                               -1,  // No file descriptor.
                                               0));

            // `MAP_FAILED` is implemented as an old-style cast on some old
            // Unix-derived platforms. Note that issuing a `#pragma GCC` here is
            // meant to cover both Clang and GCC, since the issue can manifest
            // with either toolchain. See #233.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
            if (m_buffer == MAP_FAILED)
#pragma GCC diagnostic pop
            {
                std::stringstream errorMessage;
                errorMessage << "AlignedBuffer Failed to mmap: " <<
                    std::strerror(errno) <<
                    std::endl;
                throw FatalError(errorMessage.str());
            }
#endif
            m_capacity = capacity;
        }
        else
        {
            m_usedVirtualAlloc = false;
            m_buffer = new char[capacity];
        }
    }


    void SimpleBuffer::FreeBuffer()
    {
        if (m_buffer != nullptr)
        {
            if (m_usedVirtualAlloc)
            {
#ifdef BITFUNNEL_PLATFORM_WINDOWS
                VirtualFree(m_buffer, 0, MEM_RELEASE);
#else
            if (munmap(m_buffer, m_capacity) == -1)
            {
                std::stringstream errorMessage;
                errorMessage << "AlignedBuffer Failed to mmap: " <<
                    std::strerror(errno) <<
                    std::endl;
                throw RecoverableError(errorMessage.str());
            }
#endif
            }
            else
            {
                delete [] m_buffer;
            }

            // Set m_buffer to nullptr in case subsequent code throws and causes the
            // destructor to be called.
            m_buffer = nullptr;
        }
    }
}
