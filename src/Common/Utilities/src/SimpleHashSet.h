#pragma once

#include "BitFunnel/IEnumerable.h"
#include "BitFunnel/IEnumerator.h"
#include "SimpleHashSetBase.h"


namespace BitFunnel
{
    namespace Allocators
    {
        class IAllocator;
    }

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
                          public IEnumerable<unsigned __int64>
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
                      Allocators::IAllocator& allocator);


        // Add a key to set set. If the SimpleHashSet was constructed with
        // allowResize == true and there is insufficient space to add the
        // key, Add() will trigger a reallocation of the underlying hash
        // table.
        void Add(unsigned __int64 key);

        // Returns true if the set contains the specified key.
        bool Contains(unsigned __int64 key);

        //
        // IEnumerable<unsigned __int64> methods.
        //
        // Returns an enumerator for the SimpleHashSet.  WARNING: It is the
        // caller's responsibility to delete the enumerator, even if the
        // SimpleHashSet was constructed with an arena allocator. For allocation
        // free enumeration, simple construct a SimpleHashSet::Enumerator object
        // as a local variable. GetEnumerator() exists solely to support
        // IEnumerable.
        typedef IEnumerator<unsigned __int64> Enumerator;
        Enumerator* GetEnumerator() const;

    private:
#pragma warning(push)
#pragma warning(disable:4250)
        // Returned by GetEnumerator.
        class EnumeratorObject : public SimpleHashSetBase::EnumeratorObjectBase, public IEnumerator<unsigned __int64>
        {
        public:
            EnumeratorObject(const SimpleHashSet& set);

            //
            // IEnumerator<unsigned __int64> methods.
            //
            unsigned __int64 Current() const;

        private:
            // m_set2 serves the same puporse as SimpleHashSetBase::EnumeratorObjectBase::m_set.
            // Need to put m_set2 here in EnumeratorObject to allow access to m_set.GetKey().
            // For some reason, m_set.GetKey() is inaccessible, m_set2.GetKey() is not.
            const SimpleHashSet& m_set2;
        };
#pragma warning(pop)

        // Attempts to add the specified key to the set if there is enough
        // space. Returns true on success. Otherwise returns false.
        bool AddInternal(unsigned __int64 key);

        // Expands the underlying hash table.
        void Rehash();

        // Speficies whether Add() can call Rehash() when it needs additional
        // space.
        bool m_allowResize;
    };
}
