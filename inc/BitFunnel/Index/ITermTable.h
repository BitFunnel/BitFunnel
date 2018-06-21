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

#include <iosfwd>                                   // std::ostream parameter.

#include "BitFunnel/IInterface.h"                   // Base class.
#include "BitFunnel/Index/PackedRowIdSequence.h"    // PackedRowIdSequence return value.
#include "BitFunnel/Index/RowId.h"                  // RowId parameter.
#include "BitFunnel/Term.h"                         // Term::Hash parameter.


namespace BitFunnel
{
    class ITermTable : public IInterface
    {
    public:
        //
        // TermTable build methods.
        //

        //
        // TermTable build involves
        //   1. Adding sequences of RowIds for Explicit terms.
        //   2. Adding sequences of RowIds used as recipes for Adhoc terms.
        //   3. Recording the number of Explicit, Adhoc, and Fact rows.
        //   4. Sealing the TermTable.
        //
        // Explicit terms are added as follows:
        //   Invoke OpenTerm().
        //   Call AddRowId() once for each RowId associated with the term.
        //   Invoke CloseTerm(hash) with the term's raw hash.
        //
        // Adhoc terms are added as follows:
        //   Invoke OpenTerm().
        //   Call AddRowId() once for each RowId in the recipe. These RowIds
        //     supply the combination of Rank values in the generated adhoc
        //     row. The RowIndex values are ignored as they are replaced by
        //     different hashes of the term's raw hash.
        //   Invoke CloseAdhocTerm(idf, gramSize)
        //
        // The Row counts are recorded via calls to SetRowCounts(). This method
        // should be called once for each Rank in [0..c_maxRankValue].
        //
        // Sealing is the last step, it cannot be skipped and it may only be
        // performed once. This step updates all of the stored RowIds to have
        // absolute RowIndex values.


        // Instructs the TermTable to start recording RowIds added by AddRowId.
        virtual void OpenTerm() = 0;

        // Adds a single RowId to the term table's RowId buffer.
        virtual void AddRowId(RowId id) = 0;

        // Constructs a PackedRowIdSequence from RowIds recorded since the last
        // call to OpenTerm. Stores this sequence in a map of explicit terms,
        // indexed by the supplied hash.
        virtual void CloseTerm(Term::Hash hash) = 0;

        // Constructs a PackedRowIdSequence from RowIds recorded since the last
        // call to OpenTerm. Stores this sequence in an array of adhoc term
        // recipes, indexed by the supplied idf and gramSize values.
        virtual void CloseAdhocTerm(Term::IdfX10 idf,
                                    Term::GramSize gramSize) = 0;

        // Add a mapping of a known ad hoc term's hash to its idf value
        virtual void AddAdhocTerm(Term::Hash hash, Term::IdfX10 idf) = 0;

        // Set the number of explicit and adhoc rows at each Rank.
        // Should be invoked once for rank values in [0..c_maxRankValue] during
        // TermTable build.
        virtual void SetRowCounts(Rank rank,
                                  size_t explicitCount,
                                  size_t adhocCount) = 0;

        virtual void SetFactCount(size_t factCount) = 0;

        // Completes the TermTable build process by converting relative
        // RowIndex values to absolute RowIndex values. This can only be done
        // after the row counts are set via a call to SetRowCounts().
        virtual void Seal() = 0;


        //
        // TermTable reader methods.
        //

        // Returns true if the term table has been configured to return RowIds
        // at the specified rank.
        virtual bool IsRankUsed(Rank rank) const = 0;

        // Returns the maximum rank value in any RowId, according to the
        // TermTable's present configuration.
        virtual Rank GetMaxRankUsed() const = 0;

        // Returns the total number of rows (private + shared) associated with
        // the row table for (rank). This includes rows allocated for
        // facts, if applicable.
        virtual size_t GetTotalRowCount(Rank rank) const = 0;

        // Returns the number of bytes of Row data required to store each
        // document using this TermTable.
        virtual double GetBytesPerDocument(Rank rank) const = 0;

        // Returns a PackedRowIdSequence structure associated with the
        // specified term. The PackedRowIdSequence structure contains
        // information about the term's rows. PackedRowIdSequence is used
        // by RowIdSequence to implement RowId enumeration for regular, adhoc
        // and fact terms.
        virtual PackedRowIdSequence GetRows(const Term& term) const = 0;

        // Enumeration defines the FactHandles for each of the system defined
        // terms. Used by FactSet to generate user-defined handles that don't
        // conflict with system handles.
        enum SystemTerm
        {
            DocumentActive = 0,
            MatchAll = 1,
            MatchNone = 2,
            Last = MatchNone,
            Count = 3
        };

        static Term GetDocumentActiveTerm()
        {
            return CreateSystemTerm(SystemTerm::DocumentActive);
        }

        static Term GetMatchAllTerm()
        {
            return CreateSystemTerm(SystemTerm::MatchAll);
        }

        static Term GetMatchNoneTerm()
        {
            return CreateSystemTerm(SystemTerm::MatchNone);
        }

        // Writes the contents of the ITermTable to a stream.
        virtual void Write(std::ostream& output) const = 0;

        //
        // Reader methods called by RowIdSequence::const_iterator.
        //

        // Returns a RowId from the bank of Explicit term rows.
        virtual RowId GetRowIdExplicit(size_t index) const = 0;

        // Constructs and returns a RowId according to the recipe for Adhoc
        // terms.
        virtual RowId GetRowIdAdhoc(Term::Hash hash,
                                    size_t index,
                                    size_t variant) const = 0;

        // Return the IdfX10 for an known ad hoc term
        virtual Term::IdfX10 GetIdf(Term::Hash hash) const = 0;

        virtual RowId GetRowIdFact(size_t index) const = 0;

    private:
        static Term CreateSystemTerm(SystemTerm term)
        {
            return Term(term, 0, 0);
        }
    };
}
