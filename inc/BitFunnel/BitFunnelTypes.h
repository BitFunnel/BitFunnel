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

#include <inttypes.h>   // For uint8_t, etc.
#include <limits.h>     // For constants UCHAR_MAX, _UINT32_MAX.
#include <stddef.h>     // For size_t.

namespace BitFunnel
{
    //*****************************************************************************
    //
    // Widely used typedefs and constants
    //
    //*****************************************************************************

    // IDF (inverse document frequency), multiplied by 10 and rounded to the
    // nearest integer.
    typedef uint8_t IdfX10;

    // The maximum value of IdfX10 is the value we assign to all adhoc values.
    // Note: the value UCHAR_MAX is reserved as a special parameter value in
    // class Band. IdfX10 sums may never exceed UCHAR_MAX - 1.
    const IdfX10 c_maxIdfX10Value = 70;

    // IdfSumX10 represents the sum of two IdfX10 values. IdfSumX10 is used to
    // approximate the true IdfX10 of an n-gram. It is the sum of the IdfX10
    // values corresponding to each term in the n-gram.
    typedef uint8_t IdfSumX10;

    // IdfSumX10 is an 8-bit unsigned and therefore its maximum value is
    // UCHAR_MAX.  We are reserving the top value as a flag.
    const IdfSumX10 c_maxIdfSumX10Value = UCHAR_MAX - 1;

    // A unique, system-wide document identifier.
    // For now we will make not assumptions about how DocIDs are assigned to documents.
    // The mapping may have significant and seemingly random gaps.
    // DESIGN NOTE: The concept of invalid documents (documents with invalid
    // document ID) was introduced to allow the ResultsProcessor to pass over
    // document positions used to pad the DocTable row length quanta. It also
    // provides a means to invalidate a document position after index construction.
    typedef uint64_t DocId;
    const DocId c_invalidDocId = 0;

    // A shard-independent document identifier which is local to a BitFunnel index.
    // DocIndex values in an index run from 0 to n where n-1 is the number of documents
    // in the index. The number of bits for DocIndex and ShardId together must not
    // exceed 32.
    typedef uint32_t DocIndex;

    // Represent the value that the default constructor assigns to the instances of DocumentHandle.
    static const DocIndex c_invalidDocIndex = UINT_MAX;

    // The score for a document found to match a particular query.
    // For now the score is a double. We may change the type/number of bits, and the stored data we use
    // to compute the DocScore may use less bits (e.g. the scoring table may have one byte per entry).
    typedef double DocScore;

    // A value refering to the Rank of a row or row table.  Our current
    // implementation only uses ranks 0, 3 and 6, although there are no
    // limitations on which ranks can be used.
    // DESIGN NOTE: Rank must be an unsigned type because it is used as
    // an array index.
    typedef uint32_t Rank;
    static const Rank c_maxRankValue = 6;

    // Specifies a shard in an index. A shard contains information about a set
    // of documents with similar lengths. This information includes a DocTable,
    // a ScoreTable, and one RowTable for each supported Rank.
    // The number of bits for DocIndex and ShardId together must not exceed 32.
    typedef uint32_t ShardId;
    static const unsigned c_bitsPerShardId = 4;
    static const ShardId c_maxShardCount = 1 << c_bitsPerShardId;
    static const ShardId c_maxShardValue = c_maxShardCount - 1;

    // DESIGN NOTE: the tier type (including InvalidTier) must be able to
    // fit into the 2-bit m_tier field in class RowId.
    enum Tier { DDRTier = 0, SSDTier = 1, HDDTier = 2, InvalidTier = 3, TierCount = 3, };

    // Specifies an ID of a slice of an index shard. An index shard is composed
    // of a number of Slices. A slice corresponds to a contigous range of
    // documents.
    typedef unsigned SliceId;

    // The number of words in an n-gram.
    typedef uint32_t GramSize;
    static const GramSize c_log2MaxGramSize = 3;
    static const GramSize c_maxGramSize = 1 << c_log2MaxGramSize;

    // Number of bytes in a cache line. Note that this value could go to 128 if cache line pairing
    // is enabled in the BIOS.
    const unsigned c_cacheLineRank = 6;
    const unsigned c_cacheLineMask = (1 << c_cacheLineRank) - 1;
    const unsigned c_cacheLineBytes = (1 << c_cacheLineRank);

    // Constants for alignment of different in-memory structures.
    static const size_t c_bytesPerQuadword = 8;

    // RocTable buffers are placed such that it is aligned with this
    // byte alignment. For performance reasons it is advantageous that
    // it is placed either at quadword or at cacheline boundaries.
    static const size_t c_rowTableByteAlignment = c_bytesPerQuadword;

    // DocTable buffers are placed such that it is aligned with this
    // byte alignment. For performance reasons it is advantageous that
    // it is placed either at quadword or at cacheline boundaries.
    static const size_t c_docTableByteAlignment = c_bytesPerQuadword;

    //
    // OfflinePerDocumentDataLocation alignments and constants.
    //

    // The minimum read block size from SSD is 4KB.
    // DESIGN NOTE: This value must be a multiple of 4KB.
    const size_t c_ssdReadBlockSizeInBytes = 1024 * 4;

    static_assert(c_ssdReadBlockSizeInBytes % (1024 * 4) == 0,
                  "ssdReadBlockSizeInBytes must be a multiple of 4KB.");

    // The maximal size of a per document data is capped at 1MB.
    // DESIGN NOTE: This value must be a multiple of 4KB.
    const size_t c_maximumPerDocumentDataSizeInBytes = 256 * 1024;

    static_assert(c_maximumPerDocumentDataSizeInBytes % (1024 * 4) == 0,
                  "maximumPerDocumentDataSizeInBytes must be a multiple of 4KB.");

    // Specifies the number of rows for a term.
    typedef uint8_t RowCount;

    // The number of rows reserved for the system internal purposes. These rows
    // include soft-deleted row, match-all and match-none rows.
    static const unsigned c_systemRowCount = 3;

    // The maximum gram size for terms used in false positive evaluation.
    // DESIGN NOTE: In order to have a lower SSD throughput requirement, not all gram size terms
    // are used in false positive evaluation. The optimal value for this parameter should be
    // determined based on careful designed experiments and/or simulations.
    const unsigned c_maxGramSizeForFalsePositiveEvaluation = 2;

    // The documents in the BitFunnel index can be grouped into conceptual groups
    // based on the need of the client. For example, a client can choose to group
    // documents based on time stamp. Each group is assigned a GroupId which is
    // a unique identifier for the group that the client can use to manage the
    // index.
    typedef uint64_t GroupId;

    //*************************************************************************
    //
    // Memory allocation constants.
    //
    // The following values are compile-time set to allow
    // for use of fixed size arrays, rather than vectors.
    //
    // This choice was made for performance.
    //
    //*************************************************************************

    // The largest number of rows that can be assigned to a term.
    static const RowCount c_maxRowsPerTerm = 12;

    // The maximal number of rows per query per shard.
    // TFS 567354 : Instead of hard coding a value here, make it a product of
    // c_maxRowsPerTerm and IL1Ranker::c_termPositionMax.
    // Currently it is not possible to do that because it would imply including
    // IL1Ranker.h which is not a good option.
    static const unsigned c_maxRowsPerQuery = 500;

    enum class StreamSuffixMode
    {
        IgnoreStreamSuffix,
        UseStreamSuffix
    };

    // The identifiers of values found inside DocInfo and its member variables.
    // Note: the enum values are transmitted over the wire and must not change.
    enum class ValueId
    {
        DiscoveryTimeTicks = 0,  // DocumentData::m_discoveryTimeTicks
        PublicationDateDays = 1, // DocumentData::m_publicationDateDays
    };

    //
    // NativeJIT.Library
    //
    namespace NativeJIT
    {
        template <typename T> class Node;
    }
}
