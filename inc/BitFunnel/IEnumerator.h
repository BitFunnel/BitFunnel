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

namespace BitFunnel
{
    // Polymorphic base interface for IEnumerator<T>.
    class IEnumeratorBase
    {
    public:
        virtual ~IEnumeratorBase() {};

        // Advances the enumerator to the next item in the collection. Note
        // that MoveNext() must be called at the beginning of the enumeration
        // before any calls to the Current() method. MoveNext() returns true
        // if it successfully moved to an item in the collection. MoveNext()
        // returns false when the collection is empty or the last item has
        // been passed.
        virtual bool MoveNext() = 0;

        // Moves the enumerator to the beginning at a position before the first
        // item in the collection.
        virtual void Reset() = 0;
    };

    // IEnumerator is an abstract base class for enumerating collections of
    // items of type T.
    template <typename T>
    class IEnumerator : public virtual IEnumeratorBase
    {
    public:
        virtual ~IEnumerator() {};

        // Returns the item at the current position in the enumerator. Only
        // legal if the most recent call to MoveNext() returned true.
        //
        // DESIGN NOTE: Returns T, rather than const T& to allow enumerator
        // enumerator to synthesize T values (e.g. pairs in a hash table).
        // Collections of heavy-weight items should supply enumerators of
        // pointers to avoid the large copy construction costs.
        virtual T Current() const = 0;
    };
}
