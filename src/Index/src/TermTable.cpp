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


#include <math.h>
#include <sstream>

#include "BitFunnel/BitFunnelTypes.h"
#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Utilities/StreamUtilities.h"
#include "TermTable.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // Factory methods.
    //
    //*************************************************************************
    std::unique_ptr<ITermTable> Factories::CreateTermTable()
    {
        return std::unique_ptr<ITermTable>(new TermTable());
    }


    std::unique_ptr<ITermTable>
        Factories::CreateTermTable(std::istream & input)
    {
        return std::unique_ptr<ITermTable>(new TermTable(input));
    }


    //*************************************************************************
    //
    // TermTable
    //
    //*************************************************************************
    TermTable::TermTable()
      : m_sealed(false),
        m_termOpen(false),
        m_explicitRowCounts(c_maxRankValue + 1, 0),
        m_adhocRowCounts(c_maxRankValue + 1, 0),
        m_sharedRowCounts(c_maxRankValue + 1, 0),
        m_factRowCount(SystemTerm::Count)
    {
        // Make an entry for the system rows.
        // TODO: Comment explaining why system rows are added first (rather than last).
        // Partial answer: so newly constructed TermTable is viable without a TermTableBuilder.

        OpenTerm();
        AddRowId(RowId(0, m_explicitRowCounts[0]++));
        CloseTerm(SystemTerm::DocumentActive);

        OpenTerm();
        AddRowId(RowId(0, m_explicitRowCounts[0]++));
        CloseTerm(SystemTerm::MatchAll);

        OpenTerm();
        AddRowId(RowId( 0, m_explicitRowCounts[0]++));
        CloseTerm(SystemTerm::MatchNone);
    }


    TermTable::TermTable(std::istream& input)
      : m_sealed(true),
        m_start(0)
    {
        size_t count = StreamUtilities::ReadField<size_t>(input);
        for (size_t i = 0; i < count; ++i)
        {
            const Term::Hash hash = StreamUtilities::ReadField<Term::Hash>(input);
            const PackedRowIdSequence rows = StreamUtilities::ReadField<PackedRowIdSequence>(input);

            m_termHashToRows.insert(std::make_pair(hash, rows));
        }

        m_ranksInUse = StreamUtilities::ReadField<RanksInUse>(input);
        m_maxRankInUse = StreamUtilities::ReadField<Rank>(input);
        m_adhocRows = StreamUtilities::ReadField<AdhocRecipes>(input);
        m_rowIds = StreamUtilities::ReadVector<RowId>(input);
        m_explicitRowCounts = StreamUtilities::ReadVector<RowIndex>(input);
        m_adhocRowCounts = StreamUtilities::ReadVector<RowIndex>(input);
        m_sharedRowCounts = StreamUtilities::ReadVector<RowIndex>(input);
        m_factRowCount = StreamUtilities::ReadField<RowIndex>(input);

        m_sealed = true;
    }


    void TermTable::Write(std::ostream& output) const
    {
        StreamUtilities::WriteField<size_t>(output, m_termHashToRows.size());
        for (auto entry : m_termHashToRows)
        {
            const Term::Hash hash = entry.first;
            const PackedRowIdSequence rows = entry.second;

            StreamUtilities::WriteField<Term::Hash>(output, hash);
            StreamUtilities::WriteField<PackedRowIdSequence>(output, rows);
        }

        StreamUtilities::WriteField<RanksInUse>(output, m_ranksInUse);
        StreamUtilities::WriteField<Rank>(output, m_maxRankInUse);
        StreamUtilities::WriteField<AdhocRecipes>(output, m_adhocRows);
        StreamUtilities::WriteVector(output, m_rowIds);
        StreamUtilities::WriteVector(output, m_explicitRowCounts);
        StreamUtilities::WriteVector(output, m_adhocRowCounts);
        StreamUtilities::WriteVector(output, m_sharedRowCounts);
        StreamUtilities::WriteField<RowIndex>(output, m_factRowCount);
    }

    static_assert(std::is_trivially_copyable<std::array<std::array<PackedRowIdSequence, 10>, 10>>::value, "foo");

    void TermTable::OpenTerm()
    {
        EnsureSealed(false);
        EnsureTermOpen(false);
        m_termOpen = true;
        m_start = static_cast<RowIndex>(m_rowIds.size());
    }


    void TermTable::AddRowId(RowId row)
    {
        EnsureSealed(false);
        // NOTE: we don't EnsureTermOpen because we could add system rows via AddRowId.
        m_ranksInUse[row.GetRank()] = true;
        m_rowIds.push_back(row);
    }


    // TODO: PackedRowIdSequence::Type parameter.
    void TermTable::CloseTerm(Term::Hash hash)
    {
        EnsureSealed(false);
        EnsureTermOpen(true);
        m_termOpen = false;

        // Verify that this Term::Hash hasn't been added previously.
        auto it = m_termHashToRows.find(hash);
        if (it != m_termHashToRows.end())
        {
            std::stringstream message;
            message << "TermTable::CloseTerm(): Term::Hash " << hash << " has already been added.";

            RecoverableError error(message.str());
        }

        RowIndex end = static_cast<RowIndex>(m_rowIds.size());

        m_termHashToRows.insert(
            std::make_pair(hash,
                           PackedRowIdSequence(
                               m_start,
                               end,
                               PackedRowIdSequence::Type::Explicit)));
    }


    void TermTable::CloseAdhocTerm(Term::IdfX10 idf, Term::GramSize gramSize)
    {
        EnsureSealed(false);
        EnsureTermOpen(true);
        m_termOpen = false;

        if (idf > Term::c_maxIdfX10Value || gramSize > Term::c_maxGramSize)
        {
            RecoverableError error("TermTable::CloseAdhocTerm(): parameter value out of range.");
            throw error;
        }

        RowIndex end = static_cast<RowIndex>(m_rowIds.size());

        m_adhocRows[idf][gramSize] = PackedRowIdSequence(m_start, end, PackedRowIdSequence::Type::Adhoc);
    }



    void TermTable::SetRowCounts(Rank rank,
                                 size_t explicitCount,
                                 size_t adhocCount)
    {
        EnsureSealed(false);

        m_explicitRowCounts[rank] = explicitCount;
        m_adhocRowCounts[rank] = adhocCount;
        m_sharedRowCounts[rank] = explicitCount + adhocCount;
    }


    void TermTable::SetFactCount(size_t factCount)
    {
        EnsureSealed(false);

        // Fact rows include the SystemTerm rows and one row for each user
        // defined fact.
        m_factRowCount = factCount + SystemTerm::Count;
    }


    void TermTable::Seal()
    {
        EnsureSealed(false);
        m_sealed = true;
        EnsureTermOpen(false);

        // Determine maximum rank in use.
        m_maxRankInUse = 0;
        for (Rank r = 0; r <= c_maxRankValue; ++r)
        {
            if (m_ranksInUse[r])
            {
                m_maxRankInUse = r;
            }
        }

        // Convert explicit term RowIds to use absolute RowIndex values
        // instead of values relative to the end of the block of Adhoc

        // For each explicit term.
        for (auto rows : m_termHashToRows)
        {
            // For each RowId associated with the term.
            RowIndex start = rows.second.GetStart();
            RowIndex end = rows.second.GetEnd();
            for (RowIndex r = start; r < end; ++r)
            {
                // Convert RowIndex from relative to absolute.
                RowId rowId = m_rowIds[r];
                m_rowIds[r] = RowId(rowId, m_adhocRowCounts[rowId.GetRank()]);
            }
        }
    }


    bool TermTable::IsRankUsed(Rank rank) const
    {
        return m_ranksInUse[rank];
    }


    Rank TermTable::GetMaxRankUsed() const
    {
        EnsureSealed(true);
        return m_maxRankInUse;
    }


    size_t TermTable::GetTotalRowCount(Rank rank) const
    {
        // System term rows and fact rows are included in Rank 0, but not other
        // ranks.
        auto totalRowCount = m_sharedRowCounts[rank] +
            ((rank == 0) ? m_factRowCount : 0);
        return totalRowCount;
    }


    double TermTable::GetBytesPerDocument(Rank rank) const
    {
        return GetTotalRowCount(rank) / pow(2.0, rank) / c_bitsPerByte;
    }


    PackedRowIdSequence TermTable::GetRows(const Term& term) const
    {
        const Term::Hash hash = term.GetRawHash();

        if (hash < m_factRowCount)
        {
            return PackedRowIdSequence(hash, hash + 1, PackedRowIdSequence::Type::Fact);
        }
        else
        {
            auto it = m_termHashToRows.find(term.GetRawHash());
            if (it != m_termHashToRows.end())
            {
                return (*it).second;
            }
            else
            {
                // If term isn't found, assume it is adhoc.
                // Return a PackedRowIdSequence that will be used as a recipe for
                // generating adhoc term RowIds.
                return m_adhocRows[term.GetIdfMax()][term.GetGramSize()];
            }
        }
    }


    Term TermTable::GetDocumentActiveTerm() const
    {
        return CreateSystemTerm(SystemTerm::DocumentActive);
    }


    Term TermTable::GetMatchAllTerm() const
    {
        return CreateSystemTerm(SystemTerm::MatchAll);
    }


    Term TermTable::GetMatchNoneTerm() const
    {
        return CreateSystemTerm(SystemTerm::MatchNone);
    }


    RowId TermTable::GetRowIdExplicit(size_t index) const
    {
        if (index >= m_rowIds.size())
        {
            RecoverableError error("TermTable::GetRowIdExplicit: index out of range.");
            throw error;
        }

        return m_rowIds[index];
    }


    Term TermTable::CreateSystemTerm(SystemTerm term)
    {
        return Term(term, 0, 0);
    }


    static uint64_t RotateRight(uint64_t x, uint64_t bits)
    {
        // TODO: Investigate `_rotl64` intrinsics; this exists on Windows, but
        // does not seem to on Clang.
        return (x >> bits) | (x << (64 - bits));
    }


    RowId TermTable::GetRowIdAdhoc(Term::Hash hash,
                                   size_t index,
                                   size_t variant) const
    {
        if (index >= m_rowIds.size())
        {
            RecoverableError error("TermTable::GetRowIdAdhoc: index out of range.");
            throw error;
        }

        const RowId rowId = m_rowIds[index];

        const Rank rank = rowId.GetRank();

        const size_t adhocRowCount = m_adhocRowCounts[rank];

        // Derive adhoc row index from a combination of the term hash and the
        // variant. Want to ensure that all rows generated for the same
        // (ShardId, Rank) are different.
        // Rotating by the variant gives a dramatically different number than
        // the original hash and adding in the variant ensures a different
        // value even after 64 rotations.
        hash = RotateRight(hash, variant & 0x3f) + variant;

        // Adhoc rows start at RowIndex 0.
        return RowId(rank, (hash % adhocRowCount));
    }


    RowId TermTable::GetRowIdFact(size_t index) const
    {
        if (index >= m_factRowCount)
        {
            RecoverableError error("TermTable::GetRowIdFact: index out of range.");
            throw error;
        }

        // Facts are always rank 0 and are indexed starting after the blocks of
        // adhoc and explicit. Note that the system defined terms occupy
        // the first SystemTerm::Count rows after the adhoc and explicit rows.
        // Another way of saying this is that the system terms are associated
        // with FactHandles 0..SystemTerm::Last.
        index += m_adhocRowCounts[0] + m_explicitRowCounts[0] - m_factRowCount;

        // Soft-deleted document row is the first one after all regular
        // rows. The caller specifies rowOffset = 0 in this case. The rationale
        // for this value is that a soft-deleted rowId must be consistent
        // between query planner and query runner regardless of the number of
        // user facts defined. This is to guarantee consistency of this row
        // in case of canary deployment of the code that changes the list of
        // facts.
        return RowId(0, index);
    }


    bool TermTable::operator==(TermTable const & other) const
    {
        bool equals = true;
        equals = equals && (m_ranksInUse == other.m_ranksInUse);
        equals = equals && (m_maxRankInUse == other.m_maxRankInUse);
        equals = equals && (m_termHashToRows == other.m_termHashToRows);
        equals = equals && (m_adhocRows == other.m_adhocRows);
        equals = equals && (m_rowIds == other.m_rowIds);
        equals = equals && (m_explicitRowCounts == other.m_explicitRowCounts);
        equals = equals && (m_adhocRowCounts == other.m_adhocRowCounts);
        equals = equals && (m_sharedRowCounts == other.m_sharedRowCounts);
        equals = equals && (m_factRowCount == other.m_factRowCount);

        return equals;
    }


    void TermTable::EnsureSealed(bool value) const
    {
        if (m_sealed != value)
        {
            char const * message = value ?
                "TermTable: operation disallowed before TermTable is sealed." :
                "TermTable: operation disallowed beacuse TermTable is sealed.";
            RecoverableError
                error(message);
            throw error;
        }
    }


    void TermTable::EnsureTermOpen(bool value) const
    {
        if (m_termOpen != value)
        {
            char const * message = value ?
                "TermTable: operation disallowed because Term isn't open." :
                "TermTable: operation disallowed before Term is already open.";
            RecoverableError
                error(message);
            throw error;
        }
    }
}
