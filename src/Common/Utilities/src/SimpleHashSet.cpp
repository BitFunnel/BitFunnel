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


#include <memory>           // For nullptr.

#include "BitFunnel/Allocators/IAllocator.h"
#include "LoggerInterfaces/Logging.h"
#include "SimpleHashSet.h"

namespace BitFunnel
{
    //*************************************************************************
    //
    // SimpleHashSet
    //
    //*************************************************************************
    SimpleHashSet::SimpleHashSet(unsigned capacity, bool allowResize)
        : SimpleHashSetBase(capacity, allowResize ?  c_maxProbes : capacity),
          m_allowResize(allowResize)
    {
    }


    SimpleHashSet::SimpleHashSet(unsigned capacity,
                                 bool allowResize,
                                 Allocators::IAllocator& allocator)
        : SimpleHashSetBase(capacity, allocator, allowResize ?
                            c_maxProbes :
                            capacity),
          m_allowResize(allowResize)
    {
    }


    void SimpleHashSet::Add(uint64_t key)
    {
        while (!AddInternal(key))
        {
            // We didn't find the key and were unable to allocate a slot.
            LogAssertB(m_allowResize, "HashSet overflow.");
            Rehash();
        }
    }


    bool SimpleHashSet::Contains(uint64_t key)
    {
        unsigned slot = 0;
        bool foundKey = false;
        TryFindSlot(key, slot, foundKey);

        return foundKey;
    }


    SimpleHashSet::Enumerator* SimpleHashSet::GetEnumerator() const
    {
        return new SimpleHashSet::EnumeratorObject(*this);
    }


    bool SimpleHashSet::AddInternal(uint64_t key)
    {
        unsigned slot = 0;
        bool foundKey = false;
        if (TryFindSlot(key, slot, foundKey))
        {
            if (!foundKey)
            {
                // If this is an empty slot, store the key to it.
                m_keys[slot] = key;
            }
            return true;
        }
        else
        {
            return false;
        }
    }


    void SimpleHashSet::Rehash()
    {
        unsigned oldCapacity = m_capacity;
        unsigned oldSlotCount = m_slotCount;
        uint64_t* oldKeys = ResizeKeyBuffer(m_capacity * 2);

        for (unsigned i = 0 ; i < oldSlotCount; ++i)
        {
            if (IsFilledSlot(i, oldCapacity, oldKeys))
            {
                Add(oldKeys[i]);
            }
        }

        if (m_allocator != nullptr)
        {
            m_allocator->Deallocate(oldKeys);
        }
        else
        {
            delete [] oldKeys;
        }
    }


    //*************************************************************************
    //
    // SimpleHashSet::EnumeratorObject
    //
    //*************************************************************************
    SimpleHashSet::EnumeratorObject::EnumeratorObject(const SimpleHashSet& set)
        : SimpleHashSetBase::EnumeratorObjectBase(set),
          m_set2(set)
    {
    }

    uint64_t SimpleHashSet::EnumeratorObject::Current() const
    {
        return m_set2.GetKey(m_current);
    }
}
