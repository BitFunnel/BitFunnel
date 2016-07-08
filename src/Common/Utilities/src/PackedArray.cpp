#ifdef BITFUNNEL_PLATFORM_WINDOWS
#include <Windows.h>   // For VirtualAlloc/VirtualFree.
#else
#include <cerrno>
#include <cstring>
#include <sys/mman.h>  // For mmap/munmap.
#endif

#include <memory.h>                 // For memset.

#include "BitFunnel/Utilities/FileHeader.h"
#include "BitFunnel/Utilities/StreamUtilities.h"
#include "BitFunnel/Utilities/Version.h"
#include "LoggerInterfaces/Logging.h"
#include "PackedArray.h"


namespace BitFunnel
{
    const Version PackedArray::c_version(1, 0, 0);


    PackedArray::PackedArray(size_t capacity, unsigned bitsPerEntry, bool useVirtualAlloc)
        : m_useVirtualAlloc(useVirtualAlloc),
          m_capacity(capacity),
          m_bitsPerEntry(bitsPerEntry),
          m_mask((1ULL << bitsPerEntry) - 1)
    {
        LogAssertB(bitsPerEntry > 0, "bitsPerEntry can't be 0.");
        LogAssertB(bitsPerEntry <= c_maxBitsPerEntry,
                   "bitsPerEntry out of range");

        m_buffer = AllocateBuffer(m_capacity,
                                  m_bitsPerEntry,
                                  m_useVirtualAlloc);
    }


    PackedArray::PackedArray(std::istream& input)
    {
        FileHeader fileHeader(input);
        LogAssertB(fileHeader.GetVersion().IsCompatibleWith(c_version),
                   "Incompatible PackedArray version.");

        m_capacity = StreamUtilities::ReadField<uint64_t>(input);
        m_bitsPerEntry = StreamUtilities::ReadField<uint32_t>(input);
        m_mask = (1ULL << m_bitsPerEntry) - 1;

        unsigned useVirtualAlloc = StreamUtilities::ReadField<uint32_t>(input);
        m_useVirtualAlloc = (useVirtualAlloc != 0);

        m_buffer = AllocateBuffer(m_capacity, m_bitsPerEntry, m_useVirtualAlloc);
        StreamUtilities::ReadArray<uint64_t>(input, m_buffer, GetBufferSize(m_capacity, m_bitsPerEntry));
    }


    PackedArray::~PackedArray()
    {
        if (m_useVirtualAlloc)
        {
#ifdef BITFUNNEL_PLATFORM_WINDOWS
            VirtualFree(m_buffer, 0, MEM_RELEASE);
#else
            size_t actualSize = GetBufferSize(m_capacity, m_bitsPerEntry) * sizeof(uint64_t);
            LogAssertB(munmap(m_buffer, actualSize) != -1,
                       "munmap failed %s",
                       std::strerror(errno));
#endif
        }
        else
        {
            delete [] m_buffer;
        }
    }


    void PackedArray::Write(std::ostream& output) const
    {
        FileHeader fileHeader(c_version, "PackedArray");
        fileHeader.Write(output);

        StreamUtilities::WriteField<uint64_t>(output, m_capacity);
        StreamUtilities::WriteField<uint32_t>(output, m_bitsPerEntry);
        StreamUtilities::WriteField<uint32_t>(output, m_useVirtualAlloc ? 1 : 0);
        StreamUtilities::WriteArray<uint64_t>(output, m_buffer, GetBufferSize(m_capacity, m_bitsPerEntry));
    }


    size_t PackedArray::GetCapacity() const
    {
        return m_capacity;
    }


    uint64_t PackedArray::Get(size_t index) const
    {
        LogAssertB(index < m_capacity, "PackedArray out of range.");
        return Get(index, m_bitsPerEntry, m_mask, m_buffer);
    }


    void PackedArray::Set(size_t index, uint64_t value)
    {
        LogAssertB(index < m_capacity, "PackedArray out of range.");
        Set(index, m_bitsPerEntry, m_mask, m_buffer, value);
    }


    size_t PackedArray::GetBufferSize(size_t capacity, unsigned bitsPerEntry)
    {
        LogAssertB(bitsPerEntry <= c_maxBitsPerEntry,
                   "PackedArray capacity out of range.");
        LogAssertB(capacity > 0,
                   "PackedArray with 0 capacity.");

        size_t bufferSize = (capacity * bitsPerEntry + 63) >> 6;

        // Allocate one more uint64_t so that we never have a problem
        // when loading 64-bit values that extend beyond the last valid byte.
        bufferSize++;

        return bufferSize;
    }


    uint64_t* PackedArray::AllocateBuffer(size_t capacity, unsigned bitsPerEntry, bool useVirtualAlloc)
    {
        LogAssertB(bitsPerEntry <= c_maxBitsPerEntry,
                   "PackedArray bitsPerEntry out of range.");
        LogAssertB(capacity > 0,
                   "PackedArray with 0 capacity.");

        size_t bufferSize = GetBufferSize(capacity, bitsPerEntry);
        size_t actualSize = bufferSize * sizeof(uint64_t);

        uint64_t* buffer = nullptr;
        if (useVirtualAlloc)
        {
#ifdef BITFUNNEL_PLATFORM_WINDOWS
            buffer = (uint64_t *) VirtualAlloc(nullptr,
                                               actualSize,
                                               MEM_COMMIT,
                                               PAGE_READWRITE);
            LogAssertB(buffer != nullptr,
                       "Failed to allocate buffer - error code: %x",
                       GetLastError());
#else
            buffer = static_cast<uint64_t*>(mmap(nullptr, actualSize,
                                                 PROT_READ | PROT_WRITE,
                                                 MAP_ANON | MAP_PRIVATE,
                                                 -1,  // No file descriptor.
                                                 0));
            LogAssertB(buffer != MAP_FAILED,
                       "mmap failed %s",
                       std::strerror(errno));
#endif
        }
        else
        {
            buffer = new uint64_t[bufferSize];
        }

        memset(buffer, 0, bufferSize * sizeof(uint64_t));
        return buffer;
    }


    uint64_t PackedArray::Get(size_t index,
                                      unsigned bitsPerEntry,
                                      uint64_t mask,
                                      uint64_t* buffer)
    {
        size_t bit = index * bitsPerEntry;
        size_t byte = bit >> 3;
        unsigned bitInByte = bit & 7;

        uint64_t data = *reinterpret_cast<uint64_t*>(reinterpret_cast<char*>(buffer) + byte);

        data = data >> bitInByte;
        uint64_t value = data & mask;

        return value;
    }


    void PackedArray::Set(size_t index,
                          unsigned bitsPerEntry,
                          uint64_t mask,
                          uint64_t* buffer,
                          uint64_t value)
    {
        size_t bit = index * bitsPerEntry;
        size_t byte = bit >> 3;
        unsigned bitInByte = bit & 7;

        uint64_t* ptr = reinterpret_cast<uint64_t*>(reinterpret_cast<char*>(buffer) + byte);
        uint64_t data = ~(mask << bitInByte);

        data &= *ptr;

        data |= ((value & mask) << bitInByte);
        *ptr = data;
    }


    unsigned PackedArray::GetMaxBitsPerEntry()
    {
        return c_maxBitsPerEntry;
    }
}
