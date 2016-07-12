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

// #include <istream>                        // std::istream parameter.
// #include <memory>                         // TermTable embeds std::auto_ptr.
// #include <ostream>                        // std::ostream parameter.

// #include "Array.h"                        // TermTable embeds Array3DFixed.
// #include "BitFunnel/BitFunnelTypes.h"     // For IdfX10, Shard, Tier, Rank are parameters.
// #include "BitFunnel/ITermTable.h"         // TermTable inherits from ITermTable.
// #include "BitFunnel/NonCopyable.h"        // Inherits from NonCopyable.
// #include "BitFunnel/PackedTermInfo.h"     // PackedTermInfo used as return value.
// #include "BitFunnel/RowId.h"              // For RowId used as return value.


namespace BitFunnel
{
    // namespace SimpleHashPolicy
    // {
    //     class Threadsafe;
    // }


    // template <typename T, size_t D1, size_t D2, size_t D3> class Array3DFixed;
    // template <typename T, class ThreadingPolicy> class SimpleHashTable;

    // class PackedArray;
    // class PackedTermInfo;
    // class IFactSet;
    // class ITermDisposeDefinition;
    // class Version;

    //*************************************************************************
    //
    // TermTable
    //
    // TermTable maintains a mapping from Term to PackedTermInfo for a shard. A
    // PackedTermInfo represents a set of RowId, accessible via the TermInfo
    // class. TermTable is essentially a mapping from Term to a set of RowId.
    // There is a special category of terms which correspond to facts. Facts
    // are implemented as private rank 0 rows in DDR tier. Terms which
    // correspond to facts are recognized by their raw hash value which is
    // the index of the fact in the fact set. Rows for facts are allocated
    // after shared and private rows. Serialized term table can be initialized
    // with any number of facts in IFactSet and does not require rebuild when
    // a set of fact changes.
    //
    // WARNING: Non-const methods in TermTable are not threadsafe. The const
    // methods are threadsafe in the multiple reader scenario, but are not safe
    // if there are any writers.
    //
    //*************************************************************************
    class TermTable : public ITermTable,
                      private NonCopyable
    {
    public:
    //     // Maximum allowable linear probes in hash table mapping Term::Hash
    //     // to PackedTermInfo.
    //     static const int c_maxProbes = 20;

    //     // Constructs an empty TermTable with enough space for the specified
    //     // termCount, and rowIdCapacity.
    //     TermTable(const ITermDisposeDefinition* termDisposeDefinition,
    //               unsigned termCount,
    //               unsigned rowIdCapacity);

    //     // Constructs a TermTable from data previously persisted to a stream by
    //     // the Write() method. IFactSet is used to reserve a number of rows for
    //     // recording system- and user-defined facts about a document (e.g.
    //     // document is soft-deleted, document is in top 10 etc.)
    //     TermTable(std::istream& stream, IFactSet const & facts);

    //     // Writes the TermTable data to a stream.
    //     void Write(std::ostream& stream) const;

    //     // Adds a single RowId to the term table's RowId buffer.
    //     void AddRowId(RowId id);

    //     // Returns the total number of RowIds currently in the table.
    //     // Typically used to get record a term's first RowId offset before
    //     // adding its rows.
    //     unsigned GetRowIdCount() const;

    //     // Returns the RowId at the specified offset in the TermTable.
    //     RowId GetRowId(unsigned rowOffset) const;

    //     // Returns a RowId for an adhoc term. The resulting RowId takes its
    //     // shard and tier from the RowId at the specified rowOffset. Its
    //     // RowIndex is based on a combination of the hash, the variant, and the
    //     // RowIndex in the RowId at the specified rowOffset.
    //     //
    //     // TODO: is the variant parameter really necessary? Isn't it sufficient
    //     // for the TermTableBuilder to create a set of adhoc RowIds that all
    //     // have different RowIndex values?
    //     RowId GetRowIdAdhoc(Term::Hash hash, unsigned rowOffset, unsigned variant) const;

    //     // Returns a RowId for a fact term. rowOffset specifies an index of the
    //     // fact in the list of configured facts.
    //     RowId GetRowIdForFact(unsigned rowOffset) const;

    //     // Adds a term to the term table. Rows must be added with AddRowId()
    //     // first.
    //     // TODO: Consider taking a PackedTermInfo as a parameter. This would allow
    //     // policy of Idf value selection to reside with the TermTableBuilder and
    //     // give symmetry with AddTermAdhoc.
    //     void AddTerm(Term::Hash hash, unsigned rowIdOffset, unsigned rowIdLength);

    //     // Adds an adhoc term to the term table. Rows must be added with
    //     // AddRowId() first.
    //     void AddTermAdhoc(Stream::Classification classification,
    //                       unsigned gramSize,
    //                       Tier tierHint,
    //                       IdfSumX10 idfSum,
    //                       unsigned rowIdOffset,
    //                       unsigned rowIdLength);

    //     // Setter to notify the size of different row tables
    //     void SetRowTableSize(Tier tier,
    //                          Rank rank,
    //                          unsigned privateRowCount,
    //                          unsigned sharedRowCount);

    //     // Returns the total number of rows (private + shared) associated with
    //     // the row table for (tier, rank). Note that this includes the rows
    //     // allocated for facts defined in IFactSet.
    //     virtual RowIndex GetTotalRowCount(Tier tier, Rank rank) const override;

    //     // Returns the number of rows associated with the mutable facts for
    //     // RowTables with (tier, rank).
    //     virtual RowIndex GetMutableFactRowCount(Tier tier, Rank rank) const override;

    //     // Returns a PackedTermInfo that represents the RowIds associated with
    //     // the term. The termKind output parameter indicates indicates how
    //     // the term is stored in the term table and how to get rows associated
    //     // with it. Depending on the value of this parameter, enumeration code
    //     // calls either GetRowId, GetRowIdAdhoc, GetRowIdForFact.
    //     //
    //     // DESIGN NOTE: PackedTermInfo does not store term kind in order to
    //     // save space.
    //     PackedTermInfo GetTermInfo(const Term& term, TermKind& termKind) const;

    // private:
    //     // NOTE: we keep two pointer here for different scenarios: if TermDisposeDefinition
    //     // is created by self, the auto_ptr will hold it and finally release it. Otherwise auto_ptr
    //     // is empty. Anyway, m_termDisposeDefinition points to a valid instance.
    //     std::auto_ptr<const ITermDisposeDefinition> m_termDisposeDefinitionPtr;
    //     const ITermDisposeDefinition* m_termDisposeDefinition;

    //     // Hash table stores PackedTermInfo for explicit terms (i.e. those that
    //     // are not adhoc), indexed by Term::Hash.
    //     typedef SimpleHashTable<PackedTermInfo, SimpleHashPolicy::Threadsafe> HashTable;
    //     std::auto_ptr<HashTable> m_hashToPackedTermInfo;

    //     // Storage for PackedTermInfo patterns for adhoc terms, indexed by term
    //     // classification, gram size, and tierHint property.
    //     typedef Array3DFixed<PackedTermInfo,
    //                          Stream::ClassificationCount,
    //                          c_maxGramSize + 1,
    //                          TierCount> AdhocTermInfoArray;

    //     std::auto_ptr<AdhocTermInfoArray> m_adhocTermInfos[c_maxIdfSumX10Value + 1];

    //     // Storage for RowIds referenced by PackedTermInfo structure.
    //     // PackedTermInfo structures reference contiguous sequences of RowIds
    //     // stored in m_rowIdBuffer. m_rowIdCount holds the total number of
    //     // RowIds currently stored in m_rowIdBuffer. This value is often less
    //     // than the capacity of m_rowIdBuffer.
    //     unsigned m_rowIdCount;
    //     std::auto_ptr<PackedArray> m_rowIdBuffer;

    //     typedef Array2DFixed<unsigned, TierCount, c_maxRankValue + 1>
    //     UnsignedRankTierArray;

    //     // Array of private row counts, indexed by Tier, and Rank.
    //     std::auto_ptr<UnsignedRankTierArray> m_privateRowCounts;

    //     // Array of shared row counts, indexed by Tier, and Rank.
    //     std::auto_ptr<UnsignedRankTierArray> m_sharedRowCounts;

    //     // Return this for disposed term.
    //     PackedTermInfo m_disposedTermInfo;

    //     // A number of system- and user-defined facts about documents. The
    //     // facts that are mapped to a specific region of rows in the TermTable.
    //     // These rows are not stored in the rowIdBuffer, but rather generated
    //     // at runtime using the largest RowIndex + offset value.
    //     const DocIndex m_factsCount;

    //     // Current version of TermTable.
    //     static const Version c_version;

    };
}
