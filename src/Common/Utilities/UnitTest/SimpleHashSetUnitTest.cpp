#include "stdafx.h"

#include <map>

#include "BitFunnelAllocatorInterfaces/IAllocator.h"
#include "SimpleHashSet.h"
#include "SuiteCpp/UnitTest.h"


namespace BitFunnel
{
    namespace SimpleHashSetUnitTest
    {
        class Allocator : public Allocators::IAllocator
        {
        public:
            void* Allocate(size_t size);
            void Deallocate(void* block);
            size_t MaxSize() const;
            void Reset();
        };


        void RunTest1(unsigned capacity, bool allowResize, unsigned iterations, bool useAllocator);
        static unsigned __int64 Hash(unsigned __int64 index);

        TestCase(HeapAllocations)
        {
            RunTest1(200, false, 1, false);
            RunTest1(200, false, 0, false);
            RunTest1(200, false, 100, false);
            RunTest1(200, false, 90, false);
            RunTest1(200, true, 200, false);
        }


        TestCase(ArenaAllocations)
        {
            RunTest1(200, false, 1, true);
            RunTest1(200, false, 0, true);
            RunTest1(200, false, 100, true);
            RunTest1(200, false, 90, true);
            RunTest1(200, true, 200, true);
        }


        void RunTest1(unsigned capacity, bool allowResize, unsigned iterations, bool useAllocator)
        {
            typedef std::map<unsigned __int64, unsigned> MapType;
            MapType map;

            Allocator allocator;
            std::auto_ptr<SimpleHashSet> hashSetPointer;

            if (useAllocator)
            {
                hashSetPointer.reset(new SimpleHashSet(capacity, allowResize, allocator));
            }
            else
            {
                hashSetPointer.reset(new SimpleHashSet(capacity, allowResize));
            }

            SimpleHashSet& set = *hashSetPointer;

            // For each iteration, add another key and then verify the integrity
            // and correctness of the entire contents SimpleHashSet.
            for (unsigned i = 0; i < iterations; ++i)
            {
                // Generate a random key, based on the iteration number.
                // Note that when i == 0, Hash(i) == 0 which tests the zero key case.
                unsigned __int64 key = Hash(i);
                map[key] = i;
                set.Add(key);

                // Verify that all of the keys added are in the set.
                for (MapType::const_iterator it = map.begin(); it != map.end(); ++it)
                {
                    TestAssert(set.Contains(it->first));
                }

                // Verify that all of the keys in the set are ones that were added.
                std::auto_ptr<IEnumerator<unsigned __int64>> enumerator(set.GetEnumerator());
                while (enumerator->MoveNext())
                {
                    TestAssert(map.find(enumerator->Current()) != map.end());
                }
            }
        }


        // DESIGN NOTE: It is important that Hash(0) returns 0 in order to test
        // the case where the key is 0.
        unsigned __int64 SimpleHashSetUnitTest::Hash(unsigned __int64 index)
        {
            return(index * 179426549 ^ (index * 15486953 << 32));
        }


        //*************************************************************************
        //
        // SimpleHashSetUnitTest::Allocator
        //
        //*************************************************************************
        void* SimpleHashSetUnitTest::Allocator::Allocate(size_t size)
        {
            return new char[size];
        }


        void SimpleHashSetUnitTest::Allocator::Deallocate(void* block)
        {
            delete [] block;
        }


        size_t SimpleHashSetUnitTest::Allocator::MaxSize() const
        {
            // TODO: Figure out the correct return value.
            return 10000000;
        }


        void SimpleHashSetUnitTest::Allocator::Reset()
        {
        }
    }
}
