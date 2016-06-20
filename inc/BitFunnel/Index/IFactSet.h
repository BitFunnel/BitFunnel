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

#include <stddef.h>

namespace BitFunnel
{
    //*************************************************************************
    //
    // IFactSet is an abstract base class or an interface for classes that
    // maintain a collection of FactInfo objects that describe document facts.
    // A document fact represents a predicate or assertion that is either true
    // or false for a given document, e.g. document is in top 20% static rank,
    // or document was indexed before 5pm.
    //
    // Document facts are typically implemented as private, rank 0 rows in the
    // index. Document facts may be mutable or immutable. An immutable fact for
    // a particular document is considered constant for the life of the
    // document. A mutable fact may be changed at any time.
    //
    // The IFactSet is used to configure the TermTable to be able to set and
    // clear (clear - only for mutable facts) facts about documents in the
    // index and be able to alter queries to include the facts in the query
    // plan.
    //
    //*************************************************************************
    // TODO: REVIEW:
    //   Should FactHandle be a member of IFactSet?
    //   Should IIngestionIndex provide a DefineFact() method?
    //   Should IFactSet be publicly visible?
    typedef size_t FactHandle;

    class IFactSet
    {
    public:
        virtual ~IFactSet() {};

        // Allocates a fact with sequentially numbered FactHandles starting
        // at 1.
        virtual FactHandle DefineFact(char const * friendlyName,
                                      bool isMutable) = 0;

        // Returns the number of facts defined in the IFactSet.
        virtual unsigned GetCount() const = 0;

        // Returns a handle for a fact with the specified index.
        virtual FactHandle operator[](unsigned index) const = 0;

        // Returns whether a fact with the given handle is mutable or not.
        // If a fact with this handle is not found, this function throws.
        virtual bool IsMutable(FactHandle fact) const = 0;

        // Returns a friendly name for a fact with the given handle.
        // If a fact with this handle is not found, this function throws.
        virtual char const * GetFriendlyName(FactHandle fact) const = 0;
    };
}
