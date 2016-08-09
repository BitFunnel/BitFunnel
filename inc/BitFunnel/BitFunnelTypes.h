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

#include <inttypes.h>   // For uint*_t.
#include <stddef.h>     // For size_t.


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
    // TODO: c_maxDocIndexValue? Also, should DocIndex be uint32_t since it is
    // stored?

    // TODO: remove unecessary includes of Row.h now that Rank lives here.

    // Rank is a characteristic of a row in BitFunnel.
    // Each bit in a row corresponds to a set of documents with cardinality
    // equal to 2^r where r is the Rank of the row (e.g. Rank == 0 implies
    // one document per bit, Rank == 1 implies 2 documents per bit and so on).
    // Rank is limited to fit within a three-bit field. This constraint exists
    // to bound the size of RowId to fit into 4-bytes.
    typedef size_t Rank;
    static const size_t c_log2MaxRankValue = 3;
    static const size_t c_maxRankValue = (1ul << c_log2MaxRankValue) - 1;

    // The BitFunnel index is partitioned into Shards, each of which holds
    // documents with similar sizes (e.g. shard for short documents, shard for
    // long documents).
    // ShardId is an identifier for a shard.
    // ShardId is limited to fit within a four-bit field. This constraint exists
    // to bound the size of RowId to fit into 4-bytes.
    typedef size_t ShardId;
    static const size_t c_log2MaxShardIdValue = 4;
    static const size_t c_maxShardIdValue = (1ul << c_log2MaxShardIdValue) - 1;

    // RowIndex is the ordinal position of a row in a row table. The RowIndex
    // of the first row is zero.
    // RowIndex is limited to fit within a 25-bit field. This constraint exists
    // to bound the size of RowId to fit into 4-bytes.
    typedef size_t RowIndex;
    static const size_t c_log2MaxRowIndexValue = 25;
    static const size_t c_maxRowIndexValue = (1ul << c_log2MaxRowIndexValue) - 1;
    static const size_t c_rowIndexMask = (1ul << c_log2MaxRowIndexValue) - 1;
}
