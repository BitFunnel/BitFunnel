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
    std::unique_ptr<ITermTable2> Factories::CreateTermTable()
    {
        return std::unique_ptr<ITermTable2>(new TermTable());
    }


    //*************************************************************************
    //
    // TermTable
    //
    //*************************************************************************
    TermTable::TermTable()
        : /*m_setRowCountsCalled(false),*/
          m_sealed(false),
          m_explicitRowCounts(c_maxRankValue + 1, 0),
          m_adhocRowCounts(c_maxRankValue + 1, 0),
          m_sharedRowCounts(c_maxRankValue + 1, 0),
          m_factRowCount(0)
    {
    }

    TermTable::TermTable(std::istream& input)
        : /*m_setRowCountsCalled(true),*/
        m_sealed(true),
        m_start(0)
    {
        size_t count = StreamUtilities::ReadField<size_t>(input);
        for (size_t i = 0; i < count; ++i)
        {
            const Term::Hash hash = StreamUtilities::ReadField<Term::Hash>(input);
            const PackedRowIdSequence rows = StreamUtilities::ReadField<PackedRowIdSequence>(input);

            m_termHashToRows.insert(std::make_pair(hash, rows));
        }

        m_adhocRows = StreamUtilities::ReadField<AdhocRecipes>(input);
        m_rowIds = StreamUtilities::ReadVector<RowId>(input);
        m_explicitRowCounts = StreamUtilities::ReadVector<RowIndex>(input);
        m_adhocRowCounts = StreamUtilities::ReadVector<RowIndex>(input);
        m_sharedRowCounts = StreamUtilities::ReadVector<RowIndex>(input);
        m_factRowCount = StreamUtilities::ReadField<RowIndex>(input);
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
        ThrowIfSealed();
        m_start = static_cast<RowIndex>(m_rowIds.size());
    }


    void TermTable::AddRowId(RowId row)
    {
        ThrowIfSealed();
        m_rowIds.push_back(row);
    }


    // TODO: PackedRowIdSequence::Type parameter.
    void TermTable::CloseTerm(Term::Hash hash)
    {
        ThrowIfSealed();

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
        ThrowIfSealed();

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
        ThrowIfSealed();

        m_explicitRowCounts[rank] = explicitCount;
        m_adhocRowCounts[rank] = adhocCount;
        m_sharedRowCounts[rank] = explicitCount + adhocCount;
    }


    void TermTable::Seal()
    {
        ThrowIfSealed();
        m_sealed = true;

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


    size_t TermTable::GetTotalRowCount(Rank rank) const
    {
        return
            m_sharedRowCounts[rank] +
            ((rank == 0) ? m_factRowCount : 0);
    }


    double TermTable::GetBytesPerDocument(Rank rank) const
    {
        const double c_bitsPerByte = 8.0;
        return GetTotalRowCount(rank) / pow(2.0, rank) / c_bitsPerByte;
    }


    PackedRowIdSequence TermTable::GetRows(const Term& term) const
    {
        // TODO: Implement facts.

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


    RowId TermTable::GetRowIdExplicit(size_t index) const
    {
        // TODO: Error checking - rowOffset in range?

        return m_rowIds[index];
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
        const RowId rowId = m_rowIds[index];

        const ShardId shard = rowId.GetShard();
        const Rank rank = rowId.GetRank();

        const size_t sharedRowCount = m_sharedRowCounts[rank];

        // Derive adhoc row index from a combination of the term hash and the
        // variant. Want to ensure that all rows generated for the same
        // (ShardId, Rank) are different.
        // Rotating by the variant gives a dramatically different number than
        // the original hash and adding in the variant ensures a different
        // value even after 64 rotations.
        hash = RotateRight(hash, variant & 0x3f) + variant;

        // Adhoc rows start at RowIndex 0.
        return RowId(shard, rank, (hash % sharedRowCount));
    }


    void TermTable::ThrowIfSealed() const
    {
        if (m_sealed)
        {
            RecoverableError
                error("TermTable: operation disallowed because TermTable is sealed.");
            throw error;
        }
    }
}
