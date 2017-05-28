// The MIT License (MIT)
//
// Copyright (c) 2016, Microsoft
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once

#include <iosfwd>                       // std::ostream& parameter.
//#include <iterator>                     // std::iterator base class.
#include <string>                       // std::string return value.
#include <vector>                       // std::vector embedded.

#include "BitFunnel/BitFunnelTypes.h"   // DocId return value.


namespace BitFunnel
{
    class IShardDefinition;

    //*************************************************************************
    //
    // SyntheticChunks
    //
    // Creates chunk files containing documents specially constructed to make
    // query verification trivial.
    //
    // The general strategy is that each document contains the textual
    // representations of the numbers in the range [0,n) for some value of n.
    // For example, if n=3, the document would be "0 1 2".
    //
    // One can determine the value of n for a document by dividing its DocId
    // by a scale factor. For example, if the DocId is 31 and the scale factor
    // is 4, then n will equal 7 and the document will contain
    // "0 1 2 3 4 5 6".
    //
    // The scale factor assists in generating a sufficient number of DocId
    // values for each shard. Shards for smaller documents have a smaller
    // range of legal n values. For instance, a shard of documents with unique
    // term counts in the range [32,64) only supports 32 differnt n values.
    // 
    // Suppose we want to generate 1000 documents for this shard. We do this
    // by duplicating n values. We compute a scale factor of
    //    ceiling(1000 / 32) = 32.
    //
    // Then the 1000 DocIds in the range [1024, 2024) correspond to n values
    // in the range [0,63].
    //
    // The encoding scheme for DocIds is extended further to make a document's
    // shard apparent, by encoding the shard number in the documents higher
    // order digits.
    //
    // Given a shard and scale, the DocIds for n start at
    //   shard * m_shardDocIdStart + scale * n.
    //
    //*************************************************************************
    class SyntheticChunks
    {
    public:
        SyntheticChunks(IShardDefinition const & shardDefinition,
                        size_t documentsPerShard,
                        size_t chunkCount);

        size_t GetChunkCount() const;
        void WriteChunk(std::ostream& out, size_t chunkId) const;
        std::string GetChunkName(size_t chunkId) const;

        //class const_iterator
        //    : public std::iterator<std::input_iterator_tag, DocId>
        //{
        //public:
        //    const_iterator();
        //    bool operator!=(const_iterator const & other) const;
        //    const_iterator& operator++();
        //    DocId const operator*() const;

        //private:
        //};

        //const_iterator DocIds(ShardId shard) const;
        //const_iterator end() const;

        bool Contains(DocId docId, size_t value) const;

    private:
        void WriteDocument(std::ostream& out, DocId id) const;

        // To make the index structure more apparent, DocIds in shard s fall
        // in the range [S * m_shardDocIdStart, (s + 1) * m_shardDocIdStart).
        // WARNING: Design assumes this number is a power of 10 and that it
        // is large enough that the range can accomodate all of the documents
        // in the shard.
        static const size_t m_shardDocIdStart = 100000;

        size_t m_documentsPerShard;
        size_t m_chunkCount;
        size_t m_scale;
        std::vector<size_t> m_starts;
        std::vector<size_t> m_ends;
    };
}
