#include <map>

#include "gtest/gtest.h"

#include "BitFunnel/Allocators/IAllocator.h"
#include "SimpleHashSet.h"


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
        static uint64_t Hash(uint64_t index);

        TEST(HeapAllocations, Trivial)
        {
            RunTest1(200, false, 1, false);
            RunTest1(200, false, 0, false);
            RunTest1(200, false, 100, false);
            RunTest1(200, false, 90, false);
            RunTest1(200, true, 200, false);
        }


        TEST(ArenaAllocations, Trivial)
        {
            RunTest1(200, false, 1, true);
            RunTest1(200, false, 0, true);
            RunTest1(200, false, 100, true);
            RunTest1(200, false, 90, true);
            RunTest1(200, true, 200, true);
        }


        void RunTest1(unsigned capacity, bool allowResize, unsigned iterations, bool useAllocator)
        {
            typedef std::map<uint64_t, unsigned> MapType;
            MapType map;

            Allocator allocator;
            std::unique_ptr<SimpleHashSet> hashSetPointer;

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
                uint64_t key = Hash(i);
                map[key] = i;
                set.Add(key);

                // Verify that all of the keys added are in the set.
                for (MapType::const_iterator it = map.begin(); it != map.end(); ++it)
                {
                    ASSERT_TRUE(set.Contains(it->first));
                }

                // Verify that all of the keys in the set are ones that were added.
                std::unique_ptr<IEnumerator<uint64_t>> enumerator(set.GetEnumerator());
                while (enumerator->MoveNext())
                {
                    ASSERT_TRUE(map.find(enumerator->Current()) != map.end());
                }
            }
        }


        // DESIGN NOTE: It is important that Hash(0) returns 0 in order to test
        // the case where the key is 0.
        uint64_t Hash(uint64_t index)
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
            return static_cast<void*>(new char[size]);
        }


        void SimpleHashSetUnitTest::Allocator::Deallocate(void* block)
        {
            delete static_cast<char*>(block);
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
