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

#include <algorithm>
#include <iostream>     // TODO: Remove this temporary include.
#include <math.h>
#include <ostream>

#include "BitFunnel/BitFunnelTypes.h"
#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IFactSet.h"
#include "BitFunnel/ITermTable2.h"
#include "BitFunnel/ITermTreatment.h"
#include "BitFunnel/Utilities/Stopwatch.h"
#include "DocumentFrequencyTable.h"
#include "TermTableBuilder.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // Factory methods.
    //
    //*************************************************************************
    std::unique_ptr<ITermTableBuilder>
        Factories::CreateTermTableBuilder(double density,
                                          double adhocFrequency,
                                          ITermTreatment const & treatment,
                                          IDocumentFrequencyTable const & terms,
                                          IFactSet const & facts,
                                          ITermTable2 & termTable)
    {
        return
            std::unique_ptr<ITermTableBuilder>(new TermTableBuilder(density,
                                                                    adhocFrequency,
                                                                    treatment,
                                                                    terms,
                                                                    facts,
                                                                    termTable));
    }


    //*************************************************************************
    //
    // TermTableBuilder
    //
    //*************************************************************************
    TermTableBuilder::TermTableBuilder(double density,
                                       double adhocFrequency,
                                       ITermTreatment const & treatment,
                                       IDocumentFrequencyTable const & terms,
                                       IFactSet const & facts,
                                       ITermTable2 & termTable)
        : m_termTable(termTable),
          m_buildTime(0.0)
    {
        Stopwatch stopwatch;

        // Create one RowAssigner for each rank.
        for (Rank rank = 0; rank <= c_maxRankValue; ++rank)
        {
            m_rowAssigners.push_back(
                std::unique_ptr<RowAssigner>(
                    new RowAssigner(rank,
                                    density,
                                    adhocFrequency,
                                    termTable)));
        }


        // For each entry in the document frequency table.
        // (note that the entries are sorted in order of decreasing frequency).
        for (auto dfEntry : terms)
        {
            // TODO: Consider handling disposed terms here.

            //std::cout << "Term: ";
            //dfEntry.GetTerm().Print(std::cout);
            //std::cout << "; frequency = " << dfEntry.GetFrequency() << std::endl;

            m_termTable.OpenTerm();

            // Get the term's RowConfiguration.
            auto configuration = treatment.GetTreatment(dfEntry.GetTerm());

            //std::cout << "  Configuration: ";
            //configuration.Write(std::cout);
            //std::cout << std::endl;

            // For each rank entry in the RowConfiguration.
            for (auto rcEntry : configuration)
            {
                // Assign the appropriate rows.
                m_rowAssigners[rcEntry.GetRank()]->Assign(dfEntry.GetFrequency(),
                                                          rcEntry.GetRowCount(),
                                                          rcEntry.IsPrivate());
            }

            m_termTable.CloseTerm(dfEntry.GetTerm().GetRawHash());
        }

        // TODO: make entries for facts.

        // For each (IdfX10, GramSize) pair.
        for (Term::IdfX10 idf = 0; idf <= Term::c_maxIdfX10Value; ++idf)
        {
            for (Term::GramSize gramSize = 1; gramSize < Term::c_maxGramSize; ++gramSize)
            {
                const Term::Hash hash = 0ull;
                const Term::StreamId streamId = 0;

                m_termTable.OpenTerm();

                Term term(hash, streamId, idf, gramSize);
                auto configuration = treatment.GetTreatment(term);
                for (auto rcEntry : configuration)
                {
                    for (size_t i = 0; i < rcEntry.GetRowCount(); ++i)
                    {
                        // TODO: figure out ShardId value here.
                        m_termTable.AddRowId(RowId(0, rcEntry.GetRank(), 0u));
                    }
                }

                m_termTable.CloseAdhocTerm(idf, gramSize);
            }
        }


        for (Rank rank = 0; rank <= c_maxRankValue; ++rank)
        {
            // TODO: Come up with a more principled solution.
            // When building a TermTable based on a small IDocumentFrequencyTable,
            // the builder may run into a situation where it encounters no adhoc
            // terms (because all terms encountered are in the frequency table,
            // and therefore treated as explicit).
            //
            // This could be a problem when ingesting a corpus that has adhoc terms
            // since the TermTable would not be configured with adhoc rows. A
            // TermTable with zero adhoc rows will cause a divide-by-zero when
            // attempting to compute a RowIndex by taking the mod of the hash and
            // zero.
            //
            // To avoid this problem, the TermTableBuilder always configures ranks
            // that are used in the ITermTreatment with some minimal amount of
            // adhoc rows. GetMinAdhocRowCount() returns this minimal number.
            //
            // Note that this approach also ensures there are sufficient adhoc rows
            // to greatly reduce the chances that a single adhoc term will have
            // duplicate rows.
            // https://github.com/BitFunnel/BitFunnel/issues/155

            size_t adhocRowCount = m_rowAssigners[rank]->GetAdhocRowCount();
            if (m_termTable.IsRankUsed(rank))
            {
                adhocRowCount = std::max(adhocRowCount, GetMinAdhocRowCount());
            }
            m_termTable.SetRowCounts(rank,
                                     m_rowAssigners[rank]->GetExplicitRowCount(),
                                     adhocRowCount);
        }

        m_termTable.SetFactCount(facts.GetCount());

        m_termTable.Seal();

        m_buildTime = stopwatch.ElapsedTime();
    }


    void TermTableBuilder::Print(std::ostream& output) const
    {
        output << "Total build time: " << m_buildTime << " seconds." << std::endl;

        for (auto&& assigner : m_rowAssigners)
        {
            assigner->Print(output);
        }
    }


    // TODO: Come up with a more principled solution.
    // When building a TermTable based on a small IDocumentFrequencyTable,
    // the builder may run into a situation where it encounters no adhoc
    // terms (because all terms encountered are in the frequency table,
    // and therefore treated as explicit).
    //
    // This could be a problem when ingesting a corpus that has adhoc terms
    // since the TermTable would not be configured with adhoc rows. A
    // TermTable with zero adhoc rows will cause a divide-by-zero when
    // attempting to compute a RowIndex by taking the mod of the hash and
    // zero.
    //
    // To avoid this problem, the TermTableBuilder always configures ranks
    // that are used in the ITermTreatment with some minimal amount of
    // adhoc rows. GetMinAdhocRowCount() returns this minimal number.
    //
    // Note that this approach also ensures there are sufficient adhoc rows
    // to greatly reduce the chances that a single adhoc term will have
    // duplicate rows.
    // https://github.com/BitFunnel/BitFunnel/issues/155
    size_t TermTableBuilder::GetMinAdhocRowCount()
    {
        const size_t c_minAdhocRowCount = 1000ull;
        return c_minAdhocRowCount;
    }

    //*************************************************************************
    //
    // TermTableBuilder::RowAssigner
    //
    //*************************************************************************
    TermTableBuilder::RowAssigner::RowAssigner(
        Rank rank,
        double density,
        double adhocFrequency,
        ITermTable2 & termTable)
        : m_rank(rank),
          m_density(density),
          m_adhocFrequency(adhocFrequency),
          m_termTable(termTable),
          m_adhocTotal(0),
          m_currentRow(0),
          m_explicitTermCount(0),
          m_adhocTermCount(0),
          m_privateTermCount(0),
          m_privateRowCount(0)
    {
        // TODO: Is there a way to reduce this coupling between RowAssigner
        // and the internals of TermTable?
        // Reserve first SystemTerm::Count rows for system rows like soft
        // deleted, match all, and match none.
        if (rank == 0)
        {
            m_currentRow = ITermTable2::SystemTerm::Count;
        }
    }


    void TermTableBuilder::RowAssigner::Assign(double frequency,
                                               RowIndex count,
                                               bool isPrivate)
    {
        // Compute the frequency at rank.
        double f = Term::FrequencyAtRank(frequency, m_rank);

        if (isPrivate || f >= m_density)
        {
            // A private row was requested or this term was found to have
            // sufficient frequency to warrant a private row.
            // Since the row is private (i.e. not shared with other terms)
            // we only need a single row, even if count > 1.

            ++m_privateTermCount;
            ++m_privateRowCount;

            // Just reserve the RowIndex and then add the appropriate RowID to
            // the TermTableBuilder.
            m_termTable.AddRowId(RowId(0, m_rank, m_currentRow++));
        }
        else if (f < m_adhocFrequency)
        {
            // This is an adhoc term whose location is defined by a set of
            // hash functions.
            ++m_adhocTermCount;

            // Update m_adhocTotal with this term's contribution to the total
            // number of bits.
            m_adhocTotal += frequency * count;
        }
        else
        {
            // This is an explicit term. In other words, it sets enough bits
            // that it must be bin-packed into its rows.
            ++m_explicitTermCount;

            // Use the Best Fit Descreasing bin packing algorithm.
            // See https://www.cs.ucsb.edu/~suri/cs130b/BinPacking.txt.

            // currentBins holds bins assigned for this term.
            std::vector<Bin> currentBins;

            for (RowIndex i = 0; i < count; ++i)
            {
                // Look for an existing bin with enough space.
                auto it = m_bins.lower_bound(Bin(f));
                if (it == m_bins.end())
                {
                    // No existing bin has enough space. Start a new bin.
                    currentBins.push_back(Bin(m_density, f, m_currentRow++));
                }
                else
                {
                    // Found a bin with enough space.
                    Bin bin = *it;

                    // Remove this bin from m_bins since we will be updating
                    // its space.
                    m_bins.erase(it);

                    // Reserve space in this bin for term.
                    bin.Reserve(f);

                    // Add to bins associated with this term.
                    // DESIGN NOTE: we can't immediately reinsert bin into
                    // m_bins because we must ensure that all bins for this
                    // term are unique. If we reinserted bin at this point,
                    // the call to m_bins.lower_bound() might return it on
                    // a future iteration.
                    currentBins.push_back(bin);
                }
            }

            // All of the bins for this term have been identified.

            // Now add the appropriate RowIds to the TermTable.
            for (auto b : currentBins)
            {
                // TODO: figure out ShardId value here.
                m_termTable.AddRowId(RowId(0, m_rank, b.GetIndex()));
            }

            // Reinsert the bins into m_bins.
            m_bins.insert(currentBins.begin(), currentBins.end());
        }
    }


    RowIndex TermTableBuilder::RowAssigner::GetExplicitRowCount() const
    {
        return m_currentRow;
    }


    RowIndex TermTableBuilder::RowAssigner::GetAdhocRowCount() const
    {
        double rowCount = ceil(m_adhocTotal / m_density);
        if (rowCount > c_maxRowIndexValue)
        {
            FatalError
                error("TermTableBuilder::RowAssigner::GetAdhocRowCount: too many rows.");
            throw error;
        }

        return static_cast<RowIndex>(rowCount);
    }


    void TermTableBuilder::RowAssigner::Print(std::ostream& output) const
    {
        output << "===================================" << std::endl;
        output << "RowAssigner for rank " << m_rank << std::endl;

        if (m_explicitTermCount + m_adhocTermCount == 0)
        {
            output << "  No terms" << std::endl;
        }
        else
        {
            output << "  Terms" << std::endl;
            output << "    Total: "
                   << m_adhocTermCount + m_explicitTermCount + m_privateTermCount
                   << std::endl;
            output << "    Adhoc: " << m_adhocTermCount << std::endl;
            output << "    Explicit: " << m_explicitTermCount << std::endl;
            output << "    Private: " << m_privateTermCount << std::endl;

            output << std::endl;

            output << "  Rows" << std::endl;
            output << "    Total: " << GetAdhocRowCount() + m_currentRow
                   << std::endl;
            output << "    Adhoc: " << GetAdhocRowCount() << std::endl;
            output << "    Explicit: " << m_bins.size() << std::endl;
            output << "    Private: " << m_privateRowCount << std::endl;
            output << std::endl;

            output << "  Bytes per document: "
                   << m_termTable.GetBytesPerDocument(m_rank) << std::endl;

            output << std::endl;

            Accumulator a;
            for (auto bin : m_bins)
            {
                a.Record(bin.GetFrequency(m_density));
            }

            output << std::endl;

            output << "  Densities in explicit shared rows" << std::endl;
            output << "    Mean: " << a.GetMean() << std::endl;
            output << "    Min: " << a.GetMin() << std::endl;
            output << "    Max: " << a.GetMax() << std::endl;
            output << "    Variance: " << a.GetVariance() << std::endl;
        }

        output << std::endl;
    }
}
