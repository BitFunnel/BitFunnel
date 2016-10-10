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

#include "BitFunnel/BitFunnelTypes.h"   // DocIndex return value.
#include "BitFunnel/IInterface.h"       // Base class.
#include "BitFunnel/Index/RowId.h"      // RowId parameter.


namespace BitFunnel
{

    class TermToText;

    class IShard : public IInterface
    {
    public:
        // Returns the Id of the shard.
        //virtual ShardId GetId() const = 0;

        // Returns capacity of a single Slice in the Shard. All Slices in the
        // Shard have the same capacity.
        virtual DocIndex GetSliceCapacity() const = 0;

        // Returns a vector of slice buffers for this shard.  The callers needs
        // to obtain a Token from ITokenManager to protect the pointer to the
        // list of slice buffers, as well as the buffers themselves.
        virtual std::vector<void*> const & GetSliceBuffers() const = 0;

        // Returns the offset of the row in the slice buffer in a shard.
        virtual ptrdiff_t GetRowOffset(RowId rowId) const = 0;

        // Returns the offset in the slice buffer where a pointer to the Slice
        // is stored. This is the same offset for all slices in the Shard.
        virtual ptrdiff_t GetSlicePtrOffset() const = 0;

        virtual void TemporaryWriteDocumentFrequencyTable(std::ostream& out,
                                                  TermToText const * termToText) const = 0;

    };
}
