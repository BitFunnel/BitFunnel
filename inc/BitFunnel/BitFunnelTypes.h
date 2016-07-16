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
