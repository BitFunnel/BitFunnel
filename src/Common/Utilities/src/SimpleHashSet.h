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

#include "BitFunnel/IEnumerable.h"
#include "BitFunnel/IEnumerator.h"
#include "SimpleHashSetBase.h"


namespace BitFunnel
{
    class IAllocator;


    //*************************************************************************
    //
    // SimpleHashSet provides set storage and membership predicates for keys
    // which are 64-bit hashes. Since the keys are already hashes,
    // SimpleHashSet does no hashing of its own. The initial hash table slot is
    // just a modulus of the key. Key values should contain a random
    // distribution of one and zero bits, but they don't require the guarantees
    // of a cryptographic hashing algorithm. A simpler hash like Murmurhash
    // suffices.
    //
    // SimpleHashSet is designed to allow keys to be added but not removed.
    //
    // SimpleHashSet uses linear probing to find free slots. The maximum
    // number of probes is specified by the c_maxProbes constant.
    //
    // THREAD SAFETY: This code is not threadsafe.
    //
    //*************************************************************************
    class SimpleHashSet : protected SimpleHashSetBase,
                          public IEnumerable<uint64_t>
    {
    public:
        // Constructs a SimpleHashSet with initial hash table size based
        // on the capaity param. For best performance the capacity should be
        // set to about twice the number of items to be stored in the table.
        // If allowResize is set to true, the table will automatically grow
        // as more items are added.
        SimpleHashSet(unsigned capacity, bool allowResize);

        // Constructs a SimpleHashSet where memory allocations are made from
        // a specified allocator.
        SimpleHashSet(unsigned capacity,
                      bool allowResize,
                      IAllocator& allocator);


        // Add a key to set set. If the SimpleHashSet was constructed with
        // allowResize == true and there is insufficient space to add the
        // key, Add() will trigger a reallocation of the underlying hash
        // table.
        void Add(uint64_t key);

        // Returns true if the set contains the specified key.
        bool Contains(uint64_t key);

        //
        // IEnumerable<uint64_t> methods.
        //
        // Returns an enumerator for the SimpleHashSet.  WARNING: It is the
        // caller's responsibility to delete the enumerator, even if the
        // SimpleHashSet was constructed with an arena allocator. For allocation
        // free enumeration, simple construct a SimpleHashSet::Enumerator object
        // as a local variable. GetEnumerator() exists solely to support
        // IEnumerable.
        typedef IEnumerator<uint64_t> Enumerator;
        Enumerator* GetEnumerator() const;

    private:
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4250)
#endif
        // Returned by GetEnumerator.
        class EnumeratorObject : public SimpleHashSetBase::EnumeratorObjectBase,
            public IEnumerator<uint64_t>
        {
        public:
            EnumeratorObject(const SimpleHashSet& set);

            //
            // IEnumerator<uint64_t> methods.
            //
            uint64_t Current() const;

        private:
            // m_set2 serves the same puporse as
            // SimpleHashSetBase::EnumeratorObjectBase::m_set.
            // Need to put m_set2 here in EnumeratorObject to allow access to m_set.GetKey().
            // For some reason, m_set.GetKey() is inaccessible, m_set2.GetKey() is not.
            const SimpleHashSet& m_set2;
        };
#ifdef _MSC_VER
#pragma warning(pop)
#endif

        // Attempts to add the specified key to the set if there is enough
        // space. Returns true on success. Otherwise returns false.
        bool AddInternal(uint64_t key);

        // Expands the underlying hash table.
        void Rehash();

        // Speficies whether Add() can call Rehash() when it needs additional
        // space.
        bool m_allowResize;
    };
}
