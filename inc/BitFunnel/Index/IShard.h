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

#include <cstddef>                      // ptrdiff_t return value.
#include <iosfwd>                       // std::ostream parameter.
#include "BitFunnel/BitFunnelTypes.h"   // DocIndex return value.
#include "BitFunnel/IInterface.h"       // Base class.
#include "BitFunnel/Index/RowId.h"      // RowId parameter.
#include "BitFunnel/NonCopyable.h"      // Base class.


namespace BitFunnel
{
    class DocumentHandle;
    class IFileManager;
    class ITermToText;

    class IShard : public IInterface
    {
    public:
        // Returns the Id of the shard.
        virtual ShardId GetId() const = 0;

        // Returns capacity of a single Slice in the Shard. All Slices in the
        // Shard have the same capacity.
        virtual DocIndex GetSliceCapacity() const = 0;

        // Return the size of the slice buffer in bytes.
        virtual size_t GetSliceBufferSize() const = 0;

        // Returns a vector of slice buffers for this shard.  The callers needs
        // to obtain a Token from ITokenManager to protect the pointer to the
        // list of slice buffers, as well as the buffers themselves.
        virtual std::vector<void*> const & GetSliceBuffers() const = 0;

        // Returns the offset of the row in the slice buffer in a shard.
        virtual ptrdiff_t GetRowOffset(RowId rowId) const = 0;

        virtual void TemporaryWriteDocumentFrequencyTable(
            std::ostream& out,
            ITermToText const * termToText) const = 0;

        virtual void TemporaryReadAllSlices(IFileManager& fileManager, size_t nbrSlices) = 0;

        virtual void TemporaryWriteAllSlices(IFileManager& fileManager) const = 0;


        //
        // DocumentHandle iterator
        //
        // DESIGN NOTE: Can't use the C++ <iterator> pattern because
        // iterators cannot be abstract base classes (they must be copyable)
        // and IShard cannot know the implementation of an iterator of its
        // subclass. Chose to implement a pattern that is similar to
        // the C++ iterator pattern. In this approach, const_iterator is an
        // abstract base class which is provided via and std::unique_ptr
        // from GetIterator().
        class const_iterator : public IInterface, NonCopyable
        {
        public:
            virtual const_iterator& operator++() = 0;
            virtual DocumentHandle operator*() const = 0;
            virtual bool AtEnd() const = 0;
        };

        // Returns an iterator to the DocumentHandles corresponding to
        // documents currently active in the index. This method is
        // thread safe with respect to document addition and deletion.
        // WARNING: this iterator holds a Token which will prevent
        // recycling. Be sure to release iterator when finished.
        virtual std::unique_ptr<const_iterator> GetIterator() = 0;

        // Returns an std::vector containing the bit densities for each row in
        // the RowTable with the specified rank. Bit densities are computed
        // over all slices, for those columns that correspond to active
        // documents.
        virtual std::vector<double>
            GetDensities(Rank rank) const = 0;
    };
}
