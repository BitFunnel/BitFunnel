#include "stdafx.h"

#include <map>

#include "BitFunnelAllocatorInterfaces/IAllocator.h"
#include "SimpleHashTable.h"
#include "SuiteCpp/UnitTest.h"
#include "ThrowingLogger.h"


namespace BitFunnel
{
    namespace SimpleHashPolicy
    {
        class Threadsafe;
    }

    namespace SimpleHashTableUnitTest
    {

        class Allocator : public Allocators::IAllocator
        {
        public:
            void* Allocate(size_t size);
            void Deallocate(void* block);
            size_t MaxSize() const;
            void Reset();
        };

        template <class ThreadingPolicy>
        void RunTest1(unsigned capacity, bool allowResize, unsigned iterations, bool useAllocator);

        static unsigned __int64 Hash(unsigned __int64 index);

        class Item
        {
        public:
            Item();
            Item(unsigned value);

            ~Item();

            operator unsigned() const; 

            static unsigned GetConstructionCount();
            static unsigned GetDestructionCount();
            static void ResetCounters();

        private:
            unsigned m_value;

            static unsigned m_constructionCounter;
            static unsigned m_destructionCounter;
        };

        TestCase(HeapAllocSingleThreaded)
        {
            RunTest1<SimpleHashPolicy::SingleThreaded>(200, false, 1, false);
            RunTest1<SimpleHashPolicy::SingleThreaded>(200, false, 0, false);
            RunTest1<SimpleHashPolicy::SingleThreaded>(200, false, 100, false);
            RunTest1<SimpleHashPolicy::SingleThreaded>(200, false, 90, false);
            RunTest1<SimpleHashPolicy::SingleThreaded>(200, true, 200, false);
        }


        TestCase(ArenaAllocSingleThreaded)
        {
            RunTest1<SimpleHashPolicy::SingleThreaded>(200, false, 1, true);
            RunTest1<SimpleHashPolicy::SingleThreaded>(200, false, 0, true);
            RunTest1<SimpleHashPolicy::SingleThreaded>(200, false, 100, true);
            RunTest1<SimpleHashPolicy::SingleThreaded>(200, false, 90, true);
            RunTest1<SimpleHashPolicy::SingleThreaded>(200, true, 200, true);
        }


        TestCase(HeapAllocThreadsafe)
        {
            RunTest1<SimpleHashPolicy::Threadsafe>(200, false, 1, false);
            RunTest1<SimpleHashPolicy::Threadsafe>(200, false, 0, false);
            RunTest1<SimpleHashPolicy::Threadsafe>(200, false, 100, false);
            RunTest1<SimpleHashPolicy::Threadsafe>(200, false, 90, false);

            ThrowingLogger* logger = new ThrowingLogger();
            Logging::RegisterLogger(logger);
            try
            {
                RunTest1<SimpleHashPolicy::Threadsafe>(200, true, 200, true);
                TestFail();
            }
            catch (...)
            {
            }
            Logging::RegisterLogger(nullptr);
            delete logger;
        }


        TestCase(ArenaAllocThreadsafe)
        {
            RunTest1<SimpleHashPolicy::Threadsafe>(200, false, 1, true);
            RunTest1<SimpleHashPolicy::Threadsafe>(200, false, 0, true);
            RunTest1<SimpleHashPolicy::Threadsafe>(200, false, 100, true);
            RunTest1<SimpleHashPolicy::Threadsafe>(200, false, 90, true);

            ThrowingLogger* logger = new ThrowingLogger();
            Logging::RegisterLogger(logger);
            try
            {
                RunTest1<SimpleHashPolicy::Threadsafe>(200, true, 200, true);
                TestFail();
            }
            catch (...)
            {
            }
            Logging::RegisterLogger(nullptr);
            delete logger;
        }

        
        template <class ThreadingPolicy>
        void VerifyHashTable(std::map<__int64, unsigned> const & referenceMap,
                             SimpleHashTable<Item, ThreadingPolicy> const & hashTableToCheck)
        {
            // Verify that all of the keys added are in the set.
            for (std::map<__int64, unsigned>::const_iterator it = referenceMap.begin(); it != referenceMap.end(); ++it)
            {
                bool found;
                Item& value = hashTableToCheck.Find(it->first, found);

                // Ensure the key is found.
                TestAssert(found);

                // Ensure the corrseponding value is correct.
                TestAssert(value == it->second);
            }

            // Verify that all of the keys in the set are ones that were added.
            std::auto_ptr<IEnumerator<Pair<unsigned __int64, Item>>> enumerator(hashTableToCheck.GetEnumerator());
            while (enumerator->MoveNext())
            {
                TestAssert(referenceMap.find(enumerator->Current().First()) != referenceMap.end());
            }
        }


        template <class ThreadingPolicy>
        void RunTest1(unsigned capacity,
            bool allowResize,
            unsigned iterations,
            bool useAllocator)
        {
            typedef std::map<__int64, unsigned> MapType;
            MapType map;

            Allocator allocator;
            typedef SimpleHashTable<Item, ThreadingPolicy> HashTable;
            std::auto_ptr<HashTable> tablePointer;

            Item::ResetCounters();
            if (useAllocator)
            {
                tablePointer.reset(new HashTable(capacity, allowResize, allocator));
            }
            else
            {
                tablePointer.reset(new HashTable(capacity, allowResize));
            }

            HashTable& table = *tablePointer;

            // Ensure that constructors were called when tables were initialized.
            TestAssert(Item::GetConstructionCount() == capacity + 1);
            TestAssert(Item::GetDestructionCount() == 0);
            Item::ResetCounters();

            // For each iteration, add another key and then verify the integrity
            // and correctness of the entire contents SimpleHashTable.
            for (unsigned i = 0; i < iterations; ++i)
            {
                // Generate a random key, based on the iteration number.
                // Note that when i == 0, Hash(i) == 0 which tests the zero key case.
                unsigned __int64 key = Hash(i);

                // Generate a value based on the loop counter. Added 123 to i to
                // ensure that the value for the zero key was not also zero. If the
                // zero key's value is zero, bugs relating to the special handling
                // of the zero key are less likely to be found since the values
                // array is initialized to zero.
                Item value(i + 123);
                map[key] = value;
                table[key] = value;

                VerifyHashTable(map, table);

                // Delete the key and verify.
                table.Delete(key);
                map.erase(key);

                VerifyHashTable(map, table);

                // Add the key back and verify again.
                map[key] = value;
                table[key] = value;

                VerifyHashTable(map, table);
            }

            // Check for correct number of allocations and deallocations after
            // filling the SimpleHashTable2.
            if (!allowResize || iterations < capacity)
            {
                // If we don't allow resize, or if we never added enough items
                // to trigger resize, expect one constructor call per iteration
                // and one destructor call per iteration.
                TestAssert(Item::GetConstructionCount() == iterations);
                TestAssert(Item::GetDestructionCount() == iterations);
            }
            else if (allowResize && iterations > capacity)
            {
                // If we do allow resize and we added enough items to trigger
                // at least one rehash, expect more constructor calls then
                // iterations and at least on destructor call for each slot in
                // the original table before the rehash.
                TestAssert(Item::GetConstructionCount() > iterations);
                TestAssert(Item::GetDestructionCount() >= capacity);
            }

            // Check for item destructor calls when SimpleHashTable is destroyed.
            Item::ResetCounters();
            tablePointer.reset(nullptr);
            TestAssert(Item::GetDestructionCount() >= capacity);
        }

        // DESIGN NOTE: It is important that Hash(0) returns 0 in order to test
        // the case where the key is 0.
        unsigned __int64 Hash(unsigned __int64 index)
        {
            return(index * 179426549 ^ (index * 15486953 << 32));
        }


        //*************************************************************************
        //
        // Item
        //
        //*************************************************************************
        unsigned Item::m_constructionCounter = 0;
        unsigned Item::m_destructionCounter = 0;


        Item::Item()
            : m_value(0)
        {
            ++m_constructionCounter;
        }


        Item::Item(unsigned value)
            : m_value(value)
        {
            ++m_constructionCounter;
        }


        Item::~Item()
        {
            ++m_destructionCounter;
        }


        Item::operator unsigned() const
        {
            return m_value;
        }


        unsigned Item::GetConstructionCount()
        {
            return m_constructionCounter;
        }


        unsigned Item::GetDestructionCount()
        {
            return m_destructionCounter;
        }


        void Item::ResetCounters()
        {
            m_constructionCounter = 0;
            m_destructionCounter = 0;
        }


        //*************************************************************************
        //
        // Allocator
        //
        //*************************************************************************
        void* Allocator::Allocate(size_t size)
        {
            return new char[size];
        }


        void Allocator::Deallocate(void* block)
        {
            delete [] block;
        }


        size_t Allocator::MaxSize() const
        {
            // TODO: Figure out the correct return value.
            return 10000000;
        }


        void Allocator::Reset()
        {
        }
    }
}
