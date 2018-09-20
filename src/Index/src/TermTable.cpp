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
#include "LoggerInterfaces/Check.h"
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
        m_ranksInUse({}),
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
        // Retrieve the map from all known explicit terms (hash) to row plans
        size_t count = StreamUtilities::ReadField<size_t>(input);
        for (size_t i = 0; i < count; ++i)
        {
            const Term::Hash hash = StreamUtilities::ReadField<Term::Hash>(input);
            const PackedRowIdSequence rows = StreamUtilities::ReadField<PackedRowIdSequence>(input);

            m_termHashToRows.insert(std::make_pair(hash, rows));
        }

        // Retrieve the map from known ad hoc terms (hash) to IdfX10
        // These can later be mapped to the treatment's row plan for that IdfX10
        size_t adhoccount = StreamUtilities::ReadField<size_t>(input);
        for (size_t i = 0; i < adhoccount; ++i)
        {
            const Term::Hash hash(StreamUtilities::ReadField<Term::Hash>(input));
            const Term::IdfX10 idf(StreamUtilities::ReadField<Term::IdfX10>(input));
            m_adhocTerms.insert(std::make_pair(hash, idf));
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

        StreamUtilities::WriteField<size_t>(output, m_adhocTerms.size());
        for (auto entry : m_adhocTerms)
        {
            const Term::Hash hash = entry.first;
            const Term::IdfX10 idf = entry.second;

            StreamUtilities::WriteField<Term::Hash>(output, hash);
            StreamUtilities::WriteField<Term::IdfX10>(output, idf);
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
        // NOTE: we don't EnsureTermOpen because we could add system rows via
        // AddRowId.
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

    void TermTable::AddAdhocTerm(Term::Hash hash, Term::IdfX10 idf)
    {
        EnsureSealed(false);

        // Verify that this Term::Hash hasn't been added previously.
        auto it = m_adhocTerms.find(hash);
        if (it != m_adhocTerms.end())
        {
            std::stringstream message;
            message << "TermTable::AddAdhocTerm(): Term::Hash " << hash << " has already been added.";

            RecoverableError error(message.str());
        }

        m_adhocTerms.insert(std::make_pair(hash, idf));
    }


    void TermTable::SetRowCounts(Rank rank,
                                 size_t explicitCount,
                                 size_t adhocCount)
    {
        EnsureSealed(false);

        size_t totalRowCount = explicitCount + adhocCount;
        const bool param1 = (totalRowCount != 0);   // TODO: workaround for issue #386.
        const bool param2 = m_ranksInUse[rank];     // TODO: workaround for issue #386.
        CHECK_EQ(param1, param2)
            << "inconsistent totalRowCount and m_ranksInUse.";

        m_explicitRowCounts[rank] = explicitCount;
        m_adhocRowCounts[rank] = adhocCount;
        m_sharedRowCounts[rank] = totalRowCount;
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
                //
                // Because System terms are Facts and Facts are located at the
                // end of the Explicit space to allow adding Facts after the
                // TermTable has been built, SystemTerms are relocated to the
                // end and all other Explicit terms are shifted downwards to
                // fill in the gap.
                RowId rowId = m_rowIds[r];
                if (rowId.GetRank() == 0u)
                {
                    if (r < ITermTable::SystemTerm::Count)
                    {
                        m_rowIds[r] = RowId(rowId, m_adhocRowCounts[rowId.GetRank()]
                                            + m_explicitRowCounts[0]
                                            - ITermTable::SystemTerm::Count);
                    }
                    else
                    {
                        m_rowIds[r] =
                            RowId(rowId,
                                  m_adhocRowCounts[rowId.GetRank()]
                                  - ITermTable::SystemTerm::Count);
                    }
                }
                else
                {
                    m_rowIds[r] = RowId(rowId, m_adhocRowCounts[rowId.GetRank()]);
                }
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
                return m_adhocRows[GetIdf(term.GetRawHash())][term.GetGramSize()];
            }
        }
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


    RowId TermTable::GetRowIdAdhoc(Term::Hash hash,
                                   size_t index,
                                   size_t variant) const
    {
        if (index >= m_rowIds.size())
        {
            RecoverableError error("TermTable::GetRowIdAdhoc: index out of range.");
            throw error;
        }

        if (variant > c_maxRowsPerTerm)
        {
            FatalError error("TermTable::GetRowIdAdhoc: variant out of range.");
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
        hash = hash ^ m_randomHashes[variant];

        // Adhoc rows start at RowIndex 0.
        return RowId(rank, (hash % adhocRowCount));
    }


    Term::IdfX10 TermTable::GetIdf(Term::Hash hash) const
    {
        auto it = m_adhocTerms.find(hash);
        if (it != m_adhocTerms.end())
        {
            return (*it).second;
        }
        else
        {
            return Term::c_maxIdfX10Value;
        }
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


    std::array<const Term::Hash, c_maxRandomHashes> TermTable::m_randomHashes({
        0xac0a7f8c2faac497,
        0x75a616b7c0cc21d8,
        0x43b34e9afb52a2db,
        0xc3767d8b677de5d8,
        0x09a4746cd3dea19f,
        0x155159a5f2d66662,
        0x24b70570573a2b4c,
        0x463c4be4d8bd840e,
        0x589ab2f68ccdcc45,
        0x3a392962c142487a,
        0xe67daeca274aeacf,
        0x57a86587aec8df7a,
        0x585e6b91518b8d64,
        0xa5e6f3ec194209d6,
        0x4d6b2f1248985f56,
        0x091b4e169497eea5,
        0x73082d05d013455e,
        0xf39226d5c51e08f5,
        0xfe4735c74f07ee23,
        0xaf1db9dec009bede,
        0x52bb86fa63603e79,
        0xd8a795ccb17c08cd,
        0xf38223761d033e85,
        0x93c2d0c7930ccbad,
        0x8e3b471ea7617bb8,
        0x20ddd1a3c13fff94,
        0x09cdb224b94a9189,
        0x7fd2d5f120a234c2,
        0x1fda9785cac21c1b,
        0xf448276a97e03d79,
        0xa3eab943fe79b32f,
        0xcb2d34c672aba6bc,
        0xb744c6741cd86f37,
        0x22e3849180a89d22,
        0x8068cf04a4e7fa52,
        0x355c1d9e85175126,
        0x264eb29ce80dea38,
        0xf462ef9d11f1f062,
        0x4f7999f184b110e7,
        0x69c68bae2aec2f73,
        0xbab5085c1fbaf19c,
        0x7853e16f015100e7,
        0x41f597b2e76f6a19,
        0xa9ef6a0f396845f8,
        0x2339b1aa662f34a7,
        0x77ecaeab0bbbc02b,
        0xaea1db3552dcaf5b,
        0x5b50012180f72cc0,
        0x8ee9bf5063ca9a9b,
        0x35261c5d8c4b3653,
        0x796af891aa3fd609,
        0x54304870cbc85fa2,
        0x441106fd06b37df5,
        0xc49b1f1a2f441da7,
        0x7ff27835f43793a2,
        0x83944b29ccf3cbfe,
        0x641b32a7b424f494,
        0xe8b7d7404e0f146a,
        0x8f24607794c68579,
        0xe3ac923eba5b9e9f,
        0x173bb228cfaa8756,
        0x8d8b411c7591bcac,
        0x553705a830223451,
        0x31f55f2345a641c7
    });
}
