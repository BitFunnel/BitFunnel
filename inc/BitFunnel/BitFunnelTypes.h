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

#include <inttypes.h>  // For uint*_t.
#include <stddef.h>  // For size_t.

namespace BitFunnel
{
    // TODO: remove unecessary includes of DocumentHandle.h now that DocId and
    // DocIndex live here.

    // A unique, system-wide document identifier.  For now we will make not
    // assumptions about how DocIDs are assigned to documents. The mapping may
    // have significant and seemingly random gaps.  DESIGN NOTE: The concept of
    // invalid documents (documents with invalid document ID) was introduced to
    // allow the ResultsProcessor to pass over document positions used to pad
    // the DocTable row length quanta. It also provides a means to invalidate a
    // document position after index construction.
    typedef size_t DocId;

    // A shard-independent document identifier which is local to a BitFunnel
    // index.  DocIndex values in an index run from 0 to n where n-1 is the
    // number of documents in the index. See
    // https://github.com/BitFunnel/BitFunnel/issues/53 for a potential issue
    // about the size of DocIndex and DocId.
    typedef size_t DocIndex;

    // TODO: remove unecessary includes of Row.h now that Rank lives here.

    // TODO: should this be a size_t? Although its value is always quite small.
    // If it's not a size_t, why shouldn't it be a uint8_t or something?
    typedef uint32_t Rank;

    // TODO: should this be a size_t? Although its value is always quite small.
    // If it's not a size_t, why shouldn't it be a uint8_t or something?
    typedef uint32_t ShardId;
}
