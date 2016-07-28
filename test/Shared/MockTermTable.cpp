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

#include "BitFunnel/BitFunnelTypes.h"  // For ShardId.
#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Index/IFactSet.h"
#include "BitFunnel/ITermTable.h"
#include "BitFunnel/PackedTermInfo.h"
#include "BitFunnel/RowId.h"  // For RowIndex.
#include "BitFunnel/Stream.h"
#include "LoggerInterfaces/Logging.h"
#include "MockTermTable.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // MockTermTable
    //
    //*************************************************************************
// #ifdef _MSC_VER
// #pragma warning(push)
// #pragma warning(disable:4351)
// #endif
    MockTermTable::MockTermTable(ShardId shard)
        : m_factsCount(0),
          m_privateRowCount(),
          m_shard(shard)

    {
        // Automatically add system internal rows.
        AddPrivateRowTerm(ITermTable::GetSoftDeletedTerm(), 0);
        AddPrivateRowTerm(ITermTable::GetMatchAllTerm(), 0);
        AddPrivateRowTerm(ITermTable::GetMatchNoneTerm(), 0);
    }
// #ifdef _MSC_VER
// #pragma warning(pop)
// #endif


    void MockTermTable::Write(std::ostream& /*stream*/) const
    {
        throw NotImplemented();
    }


    void MockTermTable::AddRowId(RowId id)
    {
        m_rowIds.push_back(id);
    }


    unsigned MockTermTable::GetRowIdCount() const
    {
        return static_cast<unsigned>(m_rowIds.size());
    }


    RowId MockTermTable::GetRowId(size_t rowOffset) const
    {
        return m_rowIds[rowOffset];
    }


    RowId MockTermTable::GetRowIdAdhoc(Term::Hash /*hash*/,
                                       size_t /*rowOffset*/,
                                       size_t /*variant*/)  const
    {
        throw NotImplemented();
    }


    RowId MockTermTable::GetRowIdForFact(size_t rowOffset) const
    {
        LogAssertB(rowOffset < m_factsCount,
                   "Fact rowId overflow.");
        return RowId(m_shard, 0, rowOffset);
    }


// TODO: fix sizes.
// TODO: allow non-private rows. All rows are private for now.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4267)
#endif
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#endif
    void MockTermTable::AddTerm(Term::Hash hash,
                                size_t /*rowIdOffset*/,
                                size_t rowIdLength)
    {
        LogAssertB(m_entries.find(hash) == m_entries.end(),
                   "Adding duplicate term.");
        LogAssertB(rowIdLength == 1u, "Only private rows supported.");

        Rank rank = 0;
        m_rowIds.push_back(RowId(m_shard,
                                 rank,
                                 m_privateRowCount[rank]));

        m_entries[hash] = PackedTermInfo(m_privateRowCount[rank], rowIdLength);
        ++m_privateRowCount[rank];
    }
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#ifdef __clang__
#pragma clang diagnostic pop
#endif


    void MockTermTable::SetRowTableSize(Rank /*rank*/,
                                        size_t /*rowCount*/,
                                        size_t /*sharedRowCount*/)
    {
        throw NotImplemented();
    }


    RowIndex MockTermTable::GetTotalRowCount(Rank rank) const
    {
        return m_privateRowCount[rank];
    }


    RowIndex MockTermTable::GetMutableFactRowCount(Rank /*rank*/) const
    {
        return 0;
    }


    PackedTermInfo MockTermTable::GetTermInfo(const Term& term,
                                              TermKind& termKind) const
    {
        // m_factsCount = count from IFactSet + c_systemRowCount for system
        // internal rows, hence deliberately using "less than" here.
        if (term.GetRawHash() < m_factsCount)
        {
            // Facts are private rank 0 rows with the special row offsets.
            termKind = Fact;
            const unsigned factIndex = static_cast<unsigned>(term.GetRawHash());
            return PackedTermInfo(factIndex, 1);
        }
        else
        {
            // Note: the real term table should also have adhoc (and maybe disposed) terms.
            termKind = Explicit;
            auto it = m_entries.find(term.GetRawHash());
            LogAssertB(it != m_entries.end(),
                       "Term not found.");
            return it->second;
        }
    }


    void MockTermTable::AddPrivateRowTerm(Term term, Rank /*rank*/)
    {
        // TODO: 0 is arbitrary.
        AddTerm(term.GetRawHash(), 0ull, 1);
    }


    void MockTermTable::AddRowsForFacts(IFactSet const & facts)
    {
        for (unsigned i = 0; i < facts.GetCount(); ++i)
        {
            // c_systemRowCount rows are reserved as internal.
            const Term term(static_cast<Term::Hash>(i + c_systemRowCount),
                            StreamId::MetaWord,
                            0);

            const Rank rankForFact = 0;
            ++m_factsCount;
            AddPrivateRowTerm(term, rankForFact);
        }
    }
}
