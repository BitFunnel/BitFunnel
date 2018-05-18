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

        virtual void TemporaryWriteAllSlices(IFileManager& fileManager) const = 0;

        // Get the document handle for the nth document in a shard
        // Warning: this method may not be thread-safe.
        // If the document's slice is recycled, the returned handle
        // could point to a different document than intended.
        virtual DocumentHandle GetHandle(size_t docNumber) const = 0;

        // Find next active document in shard, looking first at docNumber
        // - Return true if found and modify docNumber to position of active document
        // - Return false if no more active documents
        // Warning: this method may not be thread-safe.
        // If the document's slice is recycled, the altered docNumber
        // may no longer be valid or could reference a different document than intended.
        virtual bool FindNextActive(size_t& docNumber) const = 0;


        // Returns an std::vector containing the bit densities for each row in
        // the RowTable with the specified rank. Bit densities are computed
        // over all slices, for those columns that correspond to active
        // documents.
        virtual std::vector<double>
            GetDensities(Rank rank) const = 0;
    };
}
