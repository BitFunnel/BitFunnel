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

#include <unordered_map>            // std::unordered_map member.
#include <array>                    // std::array member.
#include <vector>                   // std::vector member.

#include "BitFunnel/ITermTable2.h"   // Base class.
#include "BitFunnel/RowId.h"        // RowId template parameter.
#include "BitFunnel/Term.h"         // Term::Hash parameter.


namespace BitFunnel
{
    class TermTable : public ITermTable2
    {
    public:
        TermTable();

        // Constructs a TermTable from data previously serialized via the
        // Write() method.
        TermTable(std::istream& input);

        // Writes the contents of the ITermTable2 to a stream.
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
        virtual Term GetSoftDeletedTerm() const override;
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

    private:
        void ThrowIfSealed() const;

        enum SystemTerm
        {
            SoftDeleted = 0,
            MatchAll = 1,
            MatchNone = 2,
            Last = MatchNone,
            Count = 3
        };

        static Term CreateSystemTerm(SystemTerm term);


        // TODO: this should actually be used.
        // bool m_setRowCountsCalled;
        bool m_sealed;

        RowIndex m_start;

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

        std::vector<RowIndex> m_explicitRowCounts;
        std::vector<RowIndex> m_adhocRowCounts;
        std::vector<RowIndex> m_sharedRowCounts;
        RowIndex m_factRowCount;
    };
}
