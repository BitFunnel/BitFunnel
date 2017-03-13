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
#include "BitFunnel/Index/ITermTable.h"
#include "BitFunnel/Index/ITermTreatment.h"
#include "BitFunnel/Utilities/Stopwatch.h"
#include "DocumentFrequencyTable.h"
#include "LoggerInterfaces/Check.h"
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
                                          ITermTable & termTable)
    {
        // TODO: make skipDistance (currently c_explicitRowRandomizaitonLimit) a parameter.
        return
            std::unique_ptr<ITermTableBuilder>(new TermTableBuilder(density,
                                                                    adhocFrequency,
                                                                    treatment,
                                                                    terms,
                                                                    facts,
                                                                    termTable,
                                                                    c_explicitRowRandomizationLimit));
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
                                       ITermTable & termTable,
                                       unsigned randomSkipDistance)
        : m_termTable(termTable),
          m_buildTime(0.0),
          // seed, min value, max value.
          m_random(std::make_unique<RandomInt<unsigned>>(0,
                                                         0,
                                                         randomSkipDistance))
    {
        Stopwatch stopwatch;

        // Create one RowAssigner for each rank.
        for (Rank rank = 0; rank <= c_maxRankValue; ++rank)
        {
            m_rowAssigners.push_back(
                std::unique_ptr<RowAssigner>(
                    new RowAssigner(rank,
                                    density,
                                    termTable,
                                    *m_random)));
        }


        // For each entry in the document frequency table.
        // (note that the entries are sorted in order of decreasing frequency).
        for (auto dfEntry : terms)
        {
            // TODO: Consider handling disposed terms here.

            //std::cout << "Term: ";
            //dfEntry.GetTerm().Print(std::cout);
            //std::cout << "; frequency = " << dfEntry.GetFrequency() << std::endl;

            // Get the term's RowConfiguration.
            auto configuration = treatment.GetTreatment(dfEntry.GetTerm());

            if (dfEntry.GetFrequency() < adhocFrequency)
            {
                // For each rank entry in the RowConfiguration.
                for (auto rcEntry : configuration)
                {
                    // Assign the appropriate rows.
                    m_rowAssigners[rcEntry.GetRank()]->
                        AssignAdhoc(dfEntry.GetFrequency(),
                                    rcEntry.GetRowCount());
                }
            }
            else
            {
                m_termTable.OpenTerm();
                //std::cout << "  Configuration: ";
                //configuration.Write(std::cout);
                //std::cout << std::endl;

                // For each rank entry in the RowConfiguration.
                for (auto rcEntry : configuration)
                {
                    // Assign the appropriate rows.
                    m_rowAssigners[rcEntry.GetRank()]->
                        AssignExplicit(dfEntry.GetFrequency(),
                                       rcEntry.GetRowCount());
                }

                m_termTable.CloseTerm(dfEntry.GetTerm().GetRawHash());
            }
        }

        // TODO: make entries for facts.

        // For each (IdfX10, GramSize) pair.
        for (Term::IdfX10 idf = 0; idf <= Term::c_maxIdfX10Value; ++idf)
        {
            // WARNING: because we don't CloseAdHocGTerm with gramSize of 0, we
            // write out unitialized memory.
            for (Term::GramSize gramSize = 1;
                 gramSize <= Term::c_maxGramSize; ++gramSize)
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
                        // Third parameter of RowId() denotes adhoc row.
                        m_termTable.AddRowId(RowId(rcEntry.GetRank(),
                                                   0u,
                                                   true));
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
    RowIndex TermTableBuilder::GetMinAdhocRowCount()
    {
        const RowIndex c_minAdhocRowCount = 1000u;
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
        ITermTable & termTable,
        RandomInt<unsigned>& random)
        : m_rank(rank),
          m_density(density),
          m_termTable(termTable),
          m_adhocTotal(0),
          m_currentRow(0),
          m_privateExplicitTermCount(0),
          m_sharedAdhocTermCount(0),
          m_sharedExplicitTermCount(0),
          m_privateExplicitRowCount(0),
          m_random(random)
    {
        // TODO: Is there a way to reduce this coupling between RowAssigner
        // and the internals of TermTable?
        // Reserve first SystemTerm::Count rows for system rows like soft
        // deleted, match all, and match none.
        if (rank == 0)
        {
            m_currentRow = ITermTable::SystemTerm::Count;
        }
    }


    void TermTableBuilder::RowAssigner::AssignExplicit(double frequency,
                                                       RowIndex count)
    {
        // Compute the frequency at rank.
        double f = Term::FrequencyAtRank(frequency, m_rank);

        if (f >= m_density)
        {
            // A private row was requested or this term was found to have
            // sufficient frequency to warrant a private row.
            // Since the row is private (i.e. not shared with other terms)
            // we only need a single row, even if count > 1.

            ++m_privateExplicitTermCount;
            ++m_privateExplicitRowCount;

            // Just reserve the RowIndex and then add the appropriate RowID to
            // the TermTableBuilder.
            m_termTable.AddRowId(RowId(m_rank, m_currentRow++));
        }
        else
        {
            // This is an explicit term. In other words, it sets enough bits
            // that it must be bin-packed into its rows.
            ++m_sharedExplicitTermCount;

            // Use the Best Fit Descreasing bin packing algorithm.
            // See https://www.cs.ucsb.edu/~suri/cs130b/BinPacking.txt.

            // currentBins holds bins assigned for this term.
            std::vector<Bin> currentBins;
            std::vector<Bin> skippedBins;

            for (RowIndex i = 0; i < count; ++i)
            {
                // Look for an existing bin with enough space.
                auto it = m_bins.lower_bound(Bin(f));
                size_t skipDistance = m_random();

                // Skip over skipDistance bins by removing them.
                //
                // In order to avoid correlations between explicitly packed
                // rows, we skip randomize row placement by skipping forward N
                // from the "optimal" placement. See
                // https://github.com/BitFunnel/BitFunnel/issues/278 for more
                // discusison. If this would skip past the end, we create a new
                // bin.

                while (it != m_bins.end() && skipDistance > 0)
                {
                    Bin bin = *it;
                    m_bins.erase(it);
                    skippedBins.push_back(bin);
                    it = m_bins.lower_bound(Bin(f));
                    --skipDistance;
                }

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

                // Add back skipped bins.
                m_bins.insert(skippedBins.begin(), skippedBins.end());
                skippedBins.clear();
            }

            // All of the bins for this term have been identified.

            // Now add the appropriate RowIds to the TermTable.
            for (auto b : currentBins)
            {
                // TODO: figure out ShardId value here.
                m_termTable.AddRowId(RowId(m_rank, b.GetIndex()));
            }

            // Reinsert the bins into m_bins.
            m_bins.insert(currentBins.begin(), currentBins.end());
        }
    }


    void TermTableBuilder::RowAssigner::AssignAdhoc(double frequency,
                                                    RowIndex count)
    {
        // This is an adhoc term whose location is defined by a set of
        // hash functions.
        ++m_sharedAdhocTermCount;

        // Update m_adhocTotal with this term's contribution to the total
        // number of bits.
        double f = Term::FrequencyAtRank(frequency, m_rank);
        m_adhocTotal += f * count;
    }


    RowIndex TermTableBuilder::RowAssigner::GetExplicitRowCount() const
    {
        return m_currentRow;
    }


    RowIndex TermTableBuilder::RowAssigner::GetAdhocRowCount() const
    {
        if (m_termTable.IsRankUsed(m_rank))
        {
            double rowCount = std::max(static_cast<double>(GetMinAdhocRowCount()),
                                       ceil(m_adhocTotal / m_density));

            CHECK_LE(rowCount, c_maxRowIndexValue)
                << "TermTableBuilder::RowAssigner::GetAdhocRowCount: too many rows.";

            return static_cast<RowIndex>(rowCount);
        }
        else
        {
            // This row table is not in use.
            return 0;
        }
    }


    void TermTableBuilder::RowAssigner::Print(std::ostream& output) const
    {
        output << "===================================" << std::endl;
        output << "RowAssigner for rank " << m_rank << std::endl;

        if (m_sharedExplicitTermCount + m_sharedAdhocTermCount + m_privateExplicitRowCount == 0)
        {
            output << "  No terms" << std::endl;
        }
        else
        {
            output << "  Terms" << std::endl;
            output << "    Total: "
                   << m_sharedAdhocTermCount + m_sharedExplicitTermCount + m_privateExplicitTermCount
                   << std::endl;
            output << "    Adhoc: " << m_sharedAdhocTermCount << std::endl;
            output << "    Explicit: " << m_sharedExplicitTermCount << std::endl;
            output << "    Private: " << m_privateExplicitTermCount << std::endl;

            output << std::endl;

            output << "  Rows" << std::endl;
            output << "    Total: " << GetAdhocRowCount() + m_currentRow
                   << std::endl;
            output << "    Adhoc: " << GetAdhocRowCount() << std::endl;
            output << "    Shared Explicit: " << m_bins.size() << std::endl;
            output << "    Private Explicit: " << m_privateExplicitRowCount << std::endl;
            output << "    Explicit: " << GetExplicitRowCount() << std::endl;
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
