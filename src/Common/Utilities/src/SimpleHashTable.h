#pragma once

#include <istream>
#include <ostream>
#include <Windows.h>                    // Template method calls 
                                        // InterlockedCompareExchange64.

#include "BitFunnelAllocatorInterfaces/IAllocator.h"     // Used in template method.
#include "BitFunnel/IEnumerable.h"      // Inherits from IEnumerable<T>.
#include "BitFunnel/IEnumerator.h"      // SimpleHashTable<T>::EnumeratorObject
                                        // inherits from IEnumerator<T>.
#include "BitFunnel/StreamUtilities.h"  // Used in template method.
#include "LoggerInterfaces/Logging.h"   // Used in template method.
#include "BitFunnel/Pair.h"             // Used as template parameter in definition
                                        // of SimpleHashTable<T>::EnumeratorObject.
#include "SimpleHashPolicy.h"           // This header is not strictly required,
                                        // but provides types that are always used
                                        // as template parameters to SimpleHashTable.
#include "SimpleHashSetBase.h"          // Inherits from SimpleHashSetBase.


// Suppress C4505 - unreferenced local function has been removed.
// This warning can crop up if you use a SimpleHashTable, but don't use all of
// its methods. Note that C4505 must be enabled for the entire header file and
// everything that comes after because the compiler generates C4505 after
// parsing all files.
#pragma warning(disable:4505)

namespace BitFunnel
{
    //*************************************************************************
    //
    // SimpleHashTable<T>
    //
    // SimpleHashTable provides (key, value) storage and access for keys which
    // are 64-bit hashes and values that are POD type T. Since the keys
    // are already hashes, SimpleHashTable does no hashing of its own. The 
    // initial hash table slot is just a modulus of the key. Key values should
    // contain a random distribution of one and zero bits, but they don't
    // require the guarantees of a cryptographic hashing algorithm. A simpler
    // hash like Murmurhash suffices.
    //
    // SimpleHashTable is designed to allow entries to be added and updated,
    // but not removed.
    //
    // SimpleHashTable uses linear probing to find free slots. The maximum
    // number of probes is specified by the c_maxProbes constant.
    //
    // THREAD SAFETY:
    //
    // SimpleHashTable<T, SimpleHashPolicy::Threadsafe> provides threadsafe and
    // nonblocking find and update operations, but is less performant than the
    // single threaded version. Note that the threadsafe version does not allow
    // hash table resizing.
    //
    // SimpleHashTable<T, SimpleHashPolicy::SingleThreaded> is not threadsafe,
    // but offers greater performance than that threadsafe version.
    //*************************************************************************
    template <typename T, class ThreadingPolicy>
    class SimpleHashTable : public SimpleHashSetBase,
                            public ThreadingPolicy,
                            public IEnumerable<Pair<unsigned __int64, T>>
    {
    public:
        // Constructs a SimpleHashTable2Base with initial hash table size based
        // on the capaity param. For best performance the capacity should be
        // set to about twice the number of items to be stored in the table.
        // If allowResize is set to true, the table will automatically grow
        // as more items are added. Note that the ThreadSafe policy does not
        // support resizing and will cause an exception in the constructor when
        // the allowResize parameter is true.
        SimpleHashTable(unsigned capacity, bool allowResize);

        // Constructs a SimpleHashTable where most memory allocations are made
        // from a specified allocator. NOTE that GetEnumerator() uses new for
        // allocation.  If allowResize is set to true, the table will
        // automatically grow as more items are added. Note that the ThreadSafe
        // policy does not support resizing and will cause an exception in the
        // constructor when the allowResize parameter is true.
        SimpleHashTable(unsigned capacity,
                        bool allowResize, 
                        Allocators::IAllocator& allocator);

        // Constructs a SimpleHashTable from data in a stream. Note that
        // SimpleHashTables constructed from a stream do not support resizing.
        SimpleHashTable(std::istream& input);

        ~SimpleHashTable();

        // Writes SimpleHashTable to a stream.
        void Write(std::ostream& output) const;

        // Returns a reference to the POD type value associated with key.
        // If key is not already in the hash table, it will be added and its
        // pointer-typed value will be initialized to nullptr. If there is not
        // enough space for the new key and resize is allowed, the key-value
        // buffers will be reallocated and rehashed.
        T& operator[](unsigned __int64 key);

        // Searches the hash table for a specified key. If the key is found,
        // the found parameter will be set to true, and a reference to the 
        // POD type value associated with the key will be returned.
        // Otherwise, the found parameter will be set to false, and an
        // unspecified reference will be returned.
        T& Find(unsigned __int64 key, bool &found) const;

        // Delete the POD type value associated with a specific key from the
        // hash table. If the key is not found, no operation is performed.
        void Delete(unsigned __int64 key);

        //
        // IEnumerable<unsigned __int64> methods.
        //
        // Returns an enumerator for the table. WARNING: It is the caller's
        // responsibility to delete the enumerator, even if the SimpleHashTable
        // was constructed with an arena allocator. For allocation-free
        // enumeration, simply construct a SimpleHashTable::Enumerator object
        // as a local variable. GetEnumerator() exists solely to support
        // IEnumerable.
        typedef IEnumerator<Pair<unsigned __int64, T>> Enumerator;
        Enumerator* GetEnumerator() const;

#pragma warning(push)
#pragma warning(disable:4250)
        class EnumeratorObject : public SimpleHashSetBase::EnumeratorObjectBase,
                                 public IEnumerator<Pair<unsigned __int64, T>>
        {
        public:
            EnumeratorObject(const SimpleHashTable<T, ThreadingPolicy>& hashTable);
            Pair<unsigned __int64, T> Current() const;

        private:
            const SimpleHashTable<T, ThreadingPolicy>& m_hashTable;
        };
#pragma warning(pop)

    private:
        // Returns the POD  type value associated with a filled slot.
        // This method will assert if the slot is invalid or empty.
        T& GetValue(unsigned slot) const;

        // Allocates and initializes the buffers containing keys and values.
        T* ResizeValueBuffer();

        // Grows the hash table buffers and rehashes all current entries into
        // the new buffers.
        void Rehash();

        // Storage for values. When the zero key is stored, m_values[m_capacity] 
        // is used to store the corresponding value.
        T* m_values;
    };


    //*************************************************************************
    //
    // Template definitions for SimpleHashTable<T>
    //
    //*************************************************************************
    template <typename T, class ThreadingPolicy>
    SimpleHashTable<T, ThreadingPolicy>::SimpleHashTable(unsigned capacity,
                                                         bool allowResize)
        : SimpleHashSetBase(capacity, allowResize ? c_maxProbes : capacity),
          ThreadingPolicy(allowResize),
          m_values(nullptr)
    {
        ResizeValueBuffer();
    }


    template <typename T, class ThreadingPolicy>
    SimpleHashTable<T, ThreadingPolicy>::SimpleHashTable(unsigned capacity,
                                                         bool allowResize,
                                                         Allocators::IAllocator& allocator)
        : SimpleHashSetBase(capacity, allocator, allowResize ? c_maxProbes : capacity),
          ThreadingPolicy(allowResize),
          m_values(nullptr)
    {
        ResizeValueBuffer();
    }


    template <typename T, class ThreadingPolicy>
    SimpleHashTable<T, ThreadingPolicy>::SimpleHashTable(std::istream& input)
        : SimpleHashSetBase(input),
          ThreadingPolicy(false),
          m_values(nullptr)
    {
        ResizeValueBuffer();
        StreamUtilities::ReadBytes(input, m_values, m_capacity * sizeof(T));
    }


    template <typename T, class ThreadingPolicy>
    SimpleHashTable<T, ThreadingPolicy>::~SimpleHashTable()
    {
        if (m_allocator != nullptr)
        {
            for (unsigned i = 0 ; i < m_slotCount; ++i)
            {
                m_values[i].~T();
            }
            m_allocator->Deallocate(m_values);
        }
        else
        {
            // TODO: Verify that this does call destructors.
            delete [] m_values;
        }
    }


    template <typename T, class ThreadingPolicy>
    void SimpleHashTable<T, ThreadingPolicy>::Write(std::ostream& output) const
    {
        SimpleHashSetBase::Write(output);
        StreamUtilities::WriteBytes(output, 
                                    reinterpret_cast<const char*>(m_values),
                                    m_capacity * sizeof(T));
    }


    template <typename T, class ThreadingPolicy>
    T& SimpleHashTable<T, ThreadingPolicy>::operator[](unsigned __int64 key)
    {
        unsigned slot = 0;
        bool foundKey = false;

        const unsigned __int64 keyForEmptySlot = GetKeyForEmptySlot(key);

        // Repeat operation until InterlockedCompareExchange64 succeeds.
        for (;;)
        {
            if (TryFindSlot(key, slot, foundKey))
            {
                if (!foundKey)
                {
                    // If this is an empty slot, allocate it by storing the key.
                    if (UpdateKeyIfUnchanged(m_keys, slot, keyForEmptySlot, key))
                    {
                        return m_values[slot];
                    }
                    // If the was no longer empty, another thread got to it,
                    // so fall out of the if and let the for-loop retry.
                }
                else
                {
                    return m_values[slot];
                }

            }
            else if (AllowsResize())
            {
                // We didn't find the key and were unable to allocate a slot.
                Rehash();

                // Attempt to find or insert the key.
                return (*this)[key];
            }
            else
            {
                LogAbortB("SimpleHashTable overflow.");
            }
        }
    }


    template <typename T, class ThreadingPolicy>
    T& SimpleHashTable<T, ThreadingPolicy>::Find(unsigned __int64 key,
                                                 bool &found) const
    {
        unsigned slot = 0;
        TryFindSlot(key, slot, found);

        return m_values[slot];
    }


    template <typename T, class ThreadingPolicy>
    void SimpleHashTable<T, ThreadingPolicy>::Delete(unsigned __int64 key)
    {
        unsigned slot = 0;
        bool foundKey = false;

        const unsigned __int64 keyForEmptySlot = GetKeyForEmptySlot(key);

        // Repeat operation until InterlockedCompareExchange64 succeeds.
        while (TryFindSlot(key, slot, foundKey)
               && foundKey
               && !UpdateKeyIfUnchanged(m_keys, slot, key, keyForEmptySlot))
        {
        }
    }


    template <typename T, class ThreadingPolicy>
    IEnumerator<Pair<unsigned __int64, T>>* SimpleHashTable<T, ThreadingPolicy>::GetEnumerator() const
    {
        return new EnumeratorObject(*this);
    }


    template <typename T, class ThreadingPolicy>
    T& SimpleHashTable<T, ThreadingPolicy>::GetValue(unsigned slot) const
    {
        LogAssertB(IsValidSlot(slot));
        LogAssertB(IsFilledSlot(slot));

        return m_values[slot];
    }


    template <typename T, class ThreadingPolicy>
    T* SimpleHashTable<T, ThreadingPolicy>::ResizeValueBuffer()
    {
        T* oldBuffer = m_values;

        if (m_allocator != nullptr)
        {
            m_values = reinterpret_cast<T*>(m_allocator->Allocate(sizeof(T) * m_slotCount));
            for (unsigned i = 0 ; i < m_slotCount; ++i)
            {
                new (m_values + i)T();
            }
        }
        else
        {
            m_values = new T[m_slotCount]();
        }

        return oldBuffer;
    }


    template <typename T, class ThreadingPolicy>
    void SimpleHashTable<T, ThreadingPolicy>::Rehash()
    {
        unsigned oldCapacity = m_capacity;
        unsigned oldSlotCount = m_slotCount;
        unsigned __int64* oldKeys = ResizeKeyBuffer(m_capacity * 2);
        T* oldValues = ResizeValueBuffer();

        for (unsigned i = 0 ; i < oldSlotCount; ++i)
        {
            if (IsFilledSlot(i, oldCapacity, oldKeys))
            {
                (*this)[oldKeys[i]] = oldValues[i];
            }
        }

        if (m_allocator != nullptr)
        {
            for (unsigned i = 0 ; i < m_slotCount; ++i)
            {
                m_values[i].~T();
            }
            m_allocator->Deallocate(oldKeys);
            m_allocator->Deallocate(oldValues);
        }
        else
        {
            // TODO: Verify destructors are called.
            delete [] oldKeys;
            delete [] oldValues;
        }
    }


    //*************************************************************************
    //
    // SimpleHashTable<T>::Enumerator
    //
    //*************************************************************************
    template <typename T, class ThreadingPolicy>
    SimpleHashTable<T, ThreadingPolicy>::EnumeratorObject::EnumeratorObject(const SimpleHashTable<T, ThreadingPolicy>& hashTable)
        : SimpleHashSetBase::EnumeratorObjectBase(hashTable),
          m_hashTable(hashTable)
    {
    }


    template <typename T, class ThreadingPolicy>
    Pair<unsigned __int64, T> SimpleHashTable<T, ThreadingPolicy>::EnumeratorObject::Current() const
    {
        return Pair<unsigned __int64, T>(m_hashTable.GetKey(m_current),
                                         m_hashTable.GetValue(m_current));
    }
}
