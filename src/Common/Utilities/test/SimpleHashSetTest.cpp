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


#include <map>
#include <memory>

#include "gtest/gtest.h"

#include "BitFunnel/Allocators/IAllocator.h"
#include "SimpleHashSet.h"


namespace BitFunnel
{
    namespace SimpleHashSetTest
    {
        class Allocator : public IAllocator
        {
        public:
            void* Allocate(size_t size);
            void Deallocate(void* block);
            size_t MaxSize() const;
            void Reset();
        };


        void RunTest1(unsigned capacity, bool allowResize, unsigned iterations,
                      bool useAllocator);
        static uint64_t Hash(uint64_t index);

        TEST(SimpleHashSet, HeapAllocations)
        {
            RunTest1(200, false, 1, false);
            RunTest1(200, false, 0, false);
            RunTest1(200, false, 100, false);
            RunTest1(200, false, 90, false);
            RunTest1(200, true, 200, false);
        }


        TEST(SimpleHashSet, ArenaAllocations)
        {
            RunTest1(200, false, 1, true);
            RunTest1(200, false, 0, true);
            RunTest1(200, false, 100, true);
            RunTest1(200, false, 90, true);
            RunTest1(200, true, 200, true);
        }


        void RunTest1(unsigned capacity, bool allowResize, unsigned iterations,
                      bool useAllocator)
        {
            typedef std::map<uint64_t, unsigned> MapType;
            MapType map;

            Allocator allocator;
            std::unique_ptr<SimpleHashSet> hashSetPointer;

            if (useAllocator)
            {
                hashSetPointer.reset(
                    new SimpleHashSet(capacity, allowResize, allocator));
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
        // SimpleHashSetTest::Allocator
        //
        //*************************************************************************
        void* SimpleHashSetTest::Allocator::Allocate(size_t size)
        {
            return static_cast<void*>(new char[size]);
        }


        void SimpleHashSetTest::Allocator::Deallocate(void* block)
        {
            delete [] static_cast<char*>(block);
        }


        size_t SimpleHashSetTest::Allocator::MaxSize() const
        {
            // TODO: Figure out the correct return value.
            return 10000000;
        }


        void SimpleHashSetTest::Allocator::Reset()
        {
        }
    }
}
