#include "stdafx.h"

#include <string>               // For memset().

#include "BitFunnelAllocatorInterfaces/IAllocator.h"
#include "BitFunnel/StreamUtilities.h"
#include "LoggerInterfaces/Logging.h"
#include "SimpleHashSetBase.h"

namespace BitFunnel
{
    SimpleHashSetBase::SimpleHashSetBase(unsigned capacity, unsigned maxProbes)
        : m_allocator(nullptr),
          m_keys(nullptr),
          m_maxProbes(maxProbes)
    {
        ResizeKeyBuffer(capacity);
    }


    SimpleHashSetBase::SimpleHashSetBase(unsigned capacity,
                                         Allocators::IAllocator& allocator,
                                         unsigned maxProbes)
        : m_allocator(&allocator),
          m_keys(nullptr),
          m_maxProbes(maxProbes)
    {
        ResizeKeyBuffer(capacity);
    }


    SimpleHashSetBase::~SimpleHashSetBase()
    {
        if (m_allocator != nullptr)
        {
            // const_cast is to get rid of volatile attribute. Destruction is not thread safe
            m_allocator->Deallocate(const_cast<unsigned __int64*>(m_keys));
        }
        else
        {
            delete [] m_keys;
        }

        // DESIGN NOTE: Do not delete m_allocator. Is is not owned by this class.
    }


    SimpleHashSetBase::SimpleHashSetBase(std::istream& input)
        : m_allocator(nullptr),
          m_keys(nullptr)
    {
        m_capacity = StreamUtilities::ReadField<unsigned __int32>(input);
        m_maxProbes = m_capacity;

        ResizeKeyBuffer(m_capacity);

        // const_cast is to get rid of volatile attribute. Construction is not thread safe
        StreamUtilities::ReadBytes(input, const_cast<unsigned __int64*>(m_keys), m_slotCount * sizeof(unsigned __int64));
    }


    void SimpleHashSetBase::Write(std::ostream& output) const
    {
        StreamUtilities::WriteField<unsigned __int32>(output, m_capacity);

        // const_cast is to get rid of volatile attribute.  File writing is not thread safe
        StreamUtilities::WriteBytes(output, reinterpret_cast<const char*>(const_cast<unsigned __int64*>(m_keys)), m_slotCount * sizeof(unsigned __int64));
    }


    unsigned SimpleHashSetBase::GetNextSlot(unsigned slot) const
    {
        for (++slot; slot < m_slotCount; ++slot)
        {
            if (IsFilledSlot(slot))
            {
                break;
            }
        }

        return slot;
    }


    bool SimpleHashSetBase::IsValidSlot(unsigned slot) const
    {
        return (slot < m_slotCount);
    }


    bool SimpleHashSetBase::IsFilledSlot(unsigned slot) const
    {
        return IsFilledSlot(slot, m_capacity, m_keys); 
    }


    bool SimpleHashSetBase::IsFilledSlot(unsigned slot, unsigned capacity, volatile unsigned __int64* keys)
    {
        return (slot < capacity) ^ (keys[slot] == 0);
    }


    unsigned __int64 SimpleHashSetBase::GetKey(unsigned slot) const
    {
        LogAssertB(IsValidSlot(slot));
        LogAssertB(IsFilledSlot(slot));

        return m_keys[slot];
    }


    bool SimpleHashSetBase::TryFindSlot(unsigned __int64 key, unsigned& slotOut, bool& foundKey) const
    {
        if (key == 0)
        {
            // Special case.
            // key == 0 is used to encode empty slots. The value for key == 0
            // is stored at slot == m_capacity.
            slotOut = m_capacity;
            foundKey = HasKeyZero();
            return true;
        }
        else
        {
            // Warning: Edits to the hash to slot mapping will necessitate regeneration of the TermTables.
            unsigned slot = key % m_capacity;
            for (unsigned i = 0; i < m_maxProbes; ++i)
            {
                if (m_keys[slot] == key)
                {
                    // We found the key. Return the corresponding value.
                    slotOut = slot;
                    foundKey = true;
                    return true;
                }
                else if (m_keys[slot] == 0)
                {
                    // We encountered an empty slot. This means the key is
                    // not in the table. Return this slot.
                    slotOut = slot;
                    foundKey = false;
                    return true;
                }
                
                // The original operation slot = (slot + 1) % m_capacity was determined to 
                // be less performant since it does a 32 bit divide.
                slot++;
                if (slot >= m_capacity)
                {
                    slot = 0;
                }
            }
        }

        foundKey = false;
        return false;
    }


    unsigned __int64* SimpleHashSetBase::ResizeKeyBuffer(unsigned capacity)
    {
        // Ensure that requested capacity is greater than zero. The current 
        // implementation relies on a non-zero capacity for the modulus 
        // operation in SimpleHashSetBase::TryFindSlot(). Also 
        // SimpleHashTable::Rehash() doubles the capacity, and therefore 
        // requires a non-zero capacity in order to actually grow the table.
        LogAssertB(capacity > 0);
        m_capacity = capacity;

        // The table has m_capacity slots, plus one extra to handle the special
        // case where key == 0.
        m_slotCount = m_capacity + 1;

        // const_cast is to get rid of volatile attribute. Resizing is not thread safe
        unsigned __int64* oldBuffer = const_cast<unsigned __int64*>(m_keys);
        if (m_allocator != nullptr)
        {
            m_keys = reinterpret_cast<unsigned __int64*>(m_allocator->Allocate(sizeof(unsigned __int64) * m_slotCount));
        }
        else
        {
            m_keys = new unsigned __int64[m_slotCount];
        }

        memset(const_cast<unsigned __int64*>(m_keys), 0, sizeof(unsigned __int64) * m_slotCount);

        // Initialize this zero key slot to a non-zero value to indicate it is empty.
        m_keys[m_capacity] = c_invalidKeyZero; 
        return oldBuffer;
    }

    
    bool SimpleHashSetBase::HasKeyZero() const
    {
        return m_keys[m_capacity] == 0;
    }


    unsigned __int64 SimpleHashSetBase::GetKeyForEmptySlot(unsigned __int64 key) const
    {
        return key == 0 ? c_invalidKeyZero : 0;
    }


    //*************************************************************************
    //
    // SimpleHashBase::EnumeratorObjectBase
    //
    //*************************************************************************
    SimpleHashSetBase::EnumeratorObjectBase::EnumeratorObjectBase(const SimpleHashSetBase& hashSet)
        : m_set(hashSet),
          m_current(c_currentBeforeStart)
    {
    }

    bool SimpleHashSetBase::EnumeratorObjectBase::MoveNext()
    {
        LogAssertB(m_set.IsValidSlot(m_current) || m_current == c_currentBeforeStart); 

        m_current = m_set.GetNextSlot(m_current);

        return m_set.IsValidSlot(m_current);
    }

    void SimpleHashSetBase::EnumeratorObjectBase::Reset()
    {
        m_current = c_currentBeforeStart; 
    }
}
