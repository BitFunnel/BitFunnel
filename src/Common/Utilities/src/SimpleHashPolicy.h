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

#include <inttypes.h>  // For uint64_t.
#include <stddef.h>    // For size_t.

namespace BitFunnel
{
    namespace SimpleHashPolicy
    {
        //*********************************************************************
        //
        // The Threadsafe policy configures the SimpleHashTable template to
        // perform threadsafe find and update operations. Note that the
        // Threadsafe policy does not support hash table resizing. Also note
        // the Threadsafe policy is less performant than the SingleThreaded
        // policy.
        //
        //*********************************************************************
        class Threadsafe
        {
        public:
            // Constructs a Threadsafe policy. Since Threadsafe does not
            // hash table resizing, the allowResize parameter must be set to
            // false. A value of true will result in an exception throw from
            // the constructor.
            Threadsafe(bool allowResize);

            // Uses interlocked compare exchange to atomically update a key in
            // a hash table slot from an existing expected value to a new desired
            // value if the value of the existing key is not changed before the
            // update operation performs. Returns false if slot of the key is
            // already updated.
            bool UpdateKeyIfUnchanged(volatile uint64_t* keys,
                                      size_t slot,
                                      uint64_t expectedCurrentKey,
                                      uint64_t desiredKey);

            // Returns true if this instance allows hash table resizing. Since
            // the Threadsafe policy does not allow hash table resizing, this
            // method will always return false.
            bool AllowsResize() const;

        private:
            bool m_allowsResize;
        };


        //*********************************************************************
        //
        // The SingleThreaded policy configures the SimpleHashTable template to
        // perform single threaded find and update operations. This policy does
        // support hash table resizing. Note that SingleThreaded is more
        // performant than ThreadSafe.
        //
        //*********************************************************************
        class SingleThreaded
        {
        public:
            // Constructs a SingleThreaded policy. The allowResize parameter
            // configures the hash table's resize policy.
            SingleThreaded(bool allowResize);

            // Non-atomic method to update a key from an existing value to a
            // desired new value in a hash table slot. Returns true if the key
            // was successfully written. Returns false if the slot doesn't have
            // the expected existing key.
            bool UpdateKeyIfUnchanged(volatile uint64_t* keys,
                                      size_t slot,
                                      uint64_t expectedCurrentKey,
                                      uint64_t desiredKey);

            // Returns true if the hash table is configured to allow resizing.
            bool AllowsResize() const;

        private:
            bool m_allowsResize;
        };
    }
}
