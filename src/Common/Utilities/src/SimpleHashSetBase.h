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


#pragma once

#include <istream>
#include <limits>  // For max size_t.
#include <ostream>

#include "BitFunnel/IEnumerable.h" // EnumeratorObjectBase inherits from IEnumerable.
#include "BitFunnel/NonCopyable.h" // SimpleHashSetBase inherits from NonCopyable.


namespace BitFunnel
{
    class IAllocator;


    //*************************************************************************
    //
    // SimpleHashSetBase is a base class for collections the provide fast
    // access to a set of 64-bit keys which are hashes. Some examples are
    // SimpleHashSet and SimpleHashTable<T>). Since the keys are already
    // hashes, SimpleHashSet does no hashing of its own. The initial hash table
    // slot is just a modulus of the key. Key values should contain a random
    // distribution of one and zero bits, but they don't require the guarantees
    // of a cryptographic hashing algorithm. A simpler hash like Murmurhash
    // suffices.
    //
    // SimpleHashSet uses linear probing to find free slots. The maximum
    // number of probes is specified by the c_maxProbes constant.
    //
    // THREAD SAFETY: This code is not threadsafe.
    //
    //*************************************************************************
    class SimpleHashSetBase
    {
        static const uint64_t c_invalidKeyZero = static_cast<uint64_t>(-1);

    public:
        // Constructs a SimpleHashSetBase with initial hash table size based
        // on the capaity param. For best performance the capacity should be
        // set to about twice the number of items to be stored in the table.
        SimpleHashSetBase(size_t capacity, unsigned maxProbes);

        // Constructs a SimpleHashSetBase where memory allocations are made
        // from a specified allocator.
        SimpleHashSetBase(size_t capacity,
                          IAllocator& allocator,
                          unsigned maxProbes);

        // Constructs a SimpleHashSetBase from data in a stream.
        SimpleHashSetBase(std::istream& input);

        ~SimpleHashSetBase();

        // Writes SimpleHashSetBase to a stream.
        void Write(std::ostream& output) const;

    protected:
        // The number of linear probes to attempt before failing to find a key.
        static const int c_maxProbes = 10;

        // Given the index of a non-empty slot, returns the index of the next
        // non-empty if one exists. Otherwise, returns an invalid slot index.
        // The IsValidSlot() method can be used to check whether an index is
        // valid.
        size_t GetNextSlot(size_t slot) const;

        // Returns true if slot is a valid index for a slot. This function is
        // primarily intended to check whether GetFirstSlot() and GetNextSlot()
        // have failed to find another filled slot.
        bool IsValidSlot(size_t slot) const;

        // Returns true if a slot is currently used to hold a (key, value)
        // pair.
        bool IsFilledSlot(size_t slot) const;

        static bool IsFilledSlot(size_t slot, size_t capacity, volatile uint64_t* keys);

        // Returns the key associated with a filled slot. This method will
        // assert if the slot is invalid or empty.
        uint64_t GetKey(size_t slot) const;

        // Attempts to find a slot associated with a specified key. If the key
        // is found, the slot will be set to the key's slot and the method will
        // return true. Otherwise the method will return false.
        bool TryFindSlot(uint64_t key, size_t& slot, bool& foundKey) const;

        // Allocates and initializes the buffer containing keys.
        // Returns the previous value of the buffer. The caller must dispose
        // of the previous buffer using a call to delete [].
        uint64_t* ResizeKeyBuffer(size_t capacity);

        // True if the hash table contains a zero key.
        bool HasKeyZero() const;

        // Returns the key of the slot when the slot is empty for a given key.
        uint64_t GetKeyForEmptySlot(uint64_t key) const;

        // Pointer to allocator, or null if no allocator was supplied. Note
        // that this allocator is not owned by the SimpleHashSetBase.
        IAllocator* m_allocator;

        // Number of slots for storing non-zero keys where the key.
        size_t m_capacity;

        // Storage for keys.
        volatile uint64_t* m_keys;

        // Maximum number of probes to allow when looking for key.
        unsigned m_maxProbes;

        // Total number of slots. Always equal to m_capacity + 1. Contains one
        // extra slot for the special case where the key is zero. Normally a
        // a non-zero value in m_keys indicates a slot is taken. For the zero key
        // slot, zero means the slot is taken. So that slot is initialized to nonzero
        // value.
        size_t m_slotCount;

        // Base class for IEnumerators implemented in subclasses of SimpleHashSetBase.
        class EnumeratorObjectBase : public virtual IEnumeratorBase, NonCopyable
        {
        public:
            EnumeratorObjectBase(const SimpleHashSetBase& hashSet);

            // IEnumeratorBase methods
            bool MoveNext();
            void Reset();

        protected:
            // Current slot number for this enumeration. Value is set to -1
            // before first call to MoveNext().
            size_t m_current;

            // The SimpleHashSetBase being enumerated.
            const SimpleHashSetBase& m_set;

        private:
            static const size_t c_currentBeforeStart =
                std::numeric_limits<size_t>::max();
        };
    };
}
