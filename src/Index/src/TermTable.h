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

#include <unordered_map>                // std::unordered_map member.
#include <array>                        // std::array member.
#include <vector>                       // std::vector member.

#include "BitFunnel/Index/ITermTable.h" // Base class.
#include "BitFunnel/Index/RowId.h"      // RowId template parameter.
#include "BitFunnel/Term.h"             // Term::Hash parameter.


namespace BitFunnel
{
    class TermTable : public ITermTable
    {
    public:
        TermTable();

        // Constructs a TermTable from data previously serialized via the
        // Write() method.
        TermTable(std::istream& input);

        // Writes the contents of the ITermTable to a stream.
        virtual void Write(std::ostream& output) const override;

        // Instructs the TermTable to start recording RowIds added by AddRowId.
        virtual void OpenTerm() override;

        // Adds a single RowId to the term table's RowId buffer.
        virtual void AddRowId(RowId id) override;

        // Constructs a PackedRowIdSequence from RowIds recorded since the last
        // call to OpenTerm. Stores this sequence in a map of explicit terms,
        // indexed by the supplied hash.
        virtual void CloseTerm(Term::Hash hash) override;

        // Constructs a PackedRowIdSequence from RowIds recorded since the last
        // call to OpenTerm. Stores this sequence in an array of adhoc term
        // recipes, indexed by the supplied idf and gramSize values.
        virtual void CloseAdhocTerm(Term::IdfX10 idf,
                                    Term::GramSize gramSize) override;

        // Set the number of explicit and adhoc rows at each Rank.
        // Should be invoked once for rank values in [0..c_maxRankValue] during
        // TermTable build.
        virtual void SetRowCounts(Rank rank,
                                  size_t explicitCount,
                                  size_t adhocCount) override;

        virtual void SetFactCount(size_t factCount) override;

        // Completes the TermTable build process by converting relative
        // RowIndex values to absolute RowIndex values. This can only be done
        // after the row counts are set via a call to SetRowCounts().
        virtual void Seal() override;

        //
        // TermTable reader methods.
        //

        // Returns true if the term table has been configured to return RowIds
        // at the specified rank.
        virtual bool IsRankUsed(Rank rank) const override;

        // Returns the maximum rank value in any RowId, according to the
        // TermTable's present configuration.
        virtual Rank GetMaxRankUsed() const override;

        // Returns the total number of rows (private + shared) associated with
        // the row table for (rank). This includes rows allocated for
        // facts, if applicable.
        virtual size_t GetTotalRowCount(Rank rank) const override;

        // Returns the number of bytes of Row data required to store each
        // document using this TermTable.
        virtual double GetBytesPerDocument(Rank rank) const override;

        // Returns a PackedRowIdSequence structure associated with the
        // specified term. The PackedRowIdSequence structure contains
        // information about the term's rows. PackedRowIdSequence is used
        // by RowIdSequence to implement RowId enumeration for regular, adhoc
        // and fact terms.
        virtual PackedRowIdSequence GetRows(const Term& term) const override;

        // Getters for system defined terms.
        virtual Term GetDocumentActiveTerm() const override;
        virtual Term GetMatchAllTerm() const override;
        virtual Term GetMatchNoneTerm() const override;

        //
        // Reader methods called by RowIdSequence::const_iterator.
        //

        // Returns a RowId from the bank of Explicit term rows.
        virtual RowId GetRowIdExplicit(size_t index) const override;

        // Constructs and returns a RowId according to the recipe for Adhoc
        // terms.
        virtual RowId GetRowIdAdhoc(Term::Hash hash,
                                    size_t index,
                                    size_t variant) const override;

        virtual RowId GetRowIdFact(size_t index) const override;

        // NOTE: Included operator== for write-to-stream/read-from-stream
        // roundtrip test. Normally one wants to do black box testing, but it
        // is hard to devise a solid black box test because it is not possible
        // for a test to enumerate all of the terms in the TermTable and all
        // of the adhoc term recipes.
        bool operator==(TermTable const & other) const;

    private:
        void EnsureSealed(bool value) const;

        // This is a helper method to catch careless bugs. There's no reason, in
        // principle, that we should necessarily enforce this -- we could, for
        // example, add n-grams by repeatedly closing the same Term with
        // additional modifications. However, we don't do that now and we've had
        // at least one (transient and quickly fixed) bug where we called
        // CloseTerm multiple times without calling OpenTerm.
        void EnsureTermOpen(bool value) const;

        static Term CreateSystemTerm(SystemTerm term);

        bool m_sealed;
        bool m_termOpen;

        RowIndex m_start;

        typedef std::array<bool, c_maxRankValue + 1> RanksInUse;
        RanksInUse m_ranksInUse{};
        Rank m_maxRankInUse;

        // TODO: Is the term table big enough that we would benefit from
        // a more compact class than std::pair<RowIndex, RowIndex>>? Would this
        // even lead to a benefit if we didn't replace std::unordered_map with
        // a better hash table? Should measure actual memory use for this data
        // structure.
        std::unordered_map<Term::Hash, PackedRowIdSequence> m_termHashToRows;

        typedef
            std::array<
                std::array<PackedRowIdSequence,
                           Term::c_maxGramSize + 1>,
                Term::c_maxIdfX10Value + 1>
            AdhocRecipes;

        // Require AdhocRecipes to be trivailly copyable to allow for binary
        // serialization.
        static_assert(std::is_trivially_copyable<AdhocRecipes>::value,
                      "TermTable: AdhocRecipes must be trivially copyable.");

        AdhocRecipes m_adhocRows;

        std::vector<RowId> m_rowIds;

        // DESIGN NOTE: m_explicitRowCounts includes facts. Facts includes
        // system terms. This is mixing together two concepts, which means that
        // some uses of m_explicitRowCounts require subtracting off
        // m_factRowCount and some don't.
        std::vector<RowIndex> m_explicitRowCounts;
        std::vector<RowIndex> m_adhocRowCounts;
        std::vector<RowIndex> m_sharedRowCounts;
        RowIndex m_factRowCount;
    };
}
