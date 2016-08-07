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

#include <iostream>     // TODO: Remove this temporary include.
#include <ostream>

#include "BitFunnel/Exceptions.h"
#include "DocumentFrequencyTable.h"
#include "ITermTreatment.h"
#include "TermTableBuilder.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // TermTableBuilder
    //
    //*************************************************************************
    TermTableBuilder::TermTableBuilder(double density,
                                       double adhocFrequency,
                                       ITermTreatment const & treatment,
                                       DocumentFrequencyTable const & terms)
    {
        RowIdInserter inserter = std::back_inserter(m_rowAssignments);

        // Create one RowAssigner for each rank.
        for (Rank rank = 0; rank <= RowId::c_maxRankValue; ++rank)
        {
            m_rowAssigners.push_back(
                std::unique_ptr<RowAssigner>(
                    new RowAssigner(rank,
                                    density,
                                    adhocFrequency,
                                    inserter)));
        }


        // For each entry in the document frequency table.
        // (note that the entries are sorted in order of decreasing frequency).
        for (auto term : terms)
        {
            std::cout << "Term: ";
            term.first.Print(std::cout);
            std::cout << "; frequency = " << term.second << std::endl;

            // Record the first row assignment slot for this term.
            size_t start = m_rowAssignments.size();
            std::cout << "  start = " << start << std::endl;

            // Get the term's RowConfiguration.
            auto configuration = treatment.GetTreatment(term.first);

            std::cout << "  Configuration: ";
            configuration.Write(std::cout);
            std::cout << std::endl;

            // For each rank entry in the RowConfiguration.
            for (auto entry : configuration)
            {
                // Assign the appropriate rows.
                m_rowAssigners[entry.GetRank()]->Assign(term.second,
                                                        entry.GetRowCount(),
                                                        entry.IsPrivate());
            }

            // Record the next row assignment slot after adding row assignments
            // for this term.
            size_t end = m_rowAssignments.size();
            std::cout << "  end = " << end << std::endl;

            if (end - start != 0)
            {
                // Row assignments were generated, so this term should be stored
                // explicitly.
                m_map.insert(std::make_pair(term.first.GetRawHash(),
                                            RowAssignment(start, end)));
            }
        }
    }


    void TermTableBuilder::Print(std::ostream& output) const
    {
        for (auto&& assigner : m_rowAssigners)
        {
            assigner->Print(output);
        }

        output << "Explicit terms across all shards: " << m_map.size() << std::endl;
        output << "RowIds across all shards: " << m_rowAssignments.size() << std::endl;
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
        TermTableBuilder::RowIdInserter inserter)
        : m_rank(rank),
          m_density(density),
          m_adhocFrequency(adhocFrequency),
          m_inserter(inserter),
          m_adhocTotal(0),
          m_currentRow(0),
          m_explicitTermCount(0),
          m_adhocTermCount(0),
          m_privateTermCount(0),
          m_privateRowCount(0)
    {
    }


    void TermTableBuilder::RowAssigner::Assign(double frequency, RowIndex count, bool isPrivate)
    {
        // Compute the frequency at rank.
        double f = 1.0 - pow(1.0 - frequency, m_rank + 1);      // TODO: Put this in shared function.

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
            m_inserter = RowId(0, m_rank, m_currentRow++);
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

            // Now add the appropriate RowIds to the TermTableBuilder.
            for (auto b : currentBins)
            {
                m_inserter = RowId(0, m_rank, b.GetIndex());
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
        if (rowCount > RowId::c_maxRowIndexValue)
        {
            FatalError
                error("TermTableBuilder::RowAssigner::GetAdhocRowCount: too many rows.");
            throw error;
        }

        return static_cast<RowIndex>(rowCount);
    }


    void TermTableBuilder::RowAssigner::Print(std::ostream& output) const
    {
        // For each rank
        //   terms
        //     count
        //   explicit rows
        //     counts: total, private, shared
        //     density: min, max, mean, variance, distribution?
        //   adhoc rows
        //     count
        output << "===================================" << std::endl;
        output << "RowAssigner for rank " << m_rank << std::endl;

        if (m_explicitTermCount + m_adhocTermCount == 0)
        {
            output << "  No terms" << std::endl;
        }
        else
        {
            output << "  Terms" << std::endl;
            output << "    Total: " << m_adhocTermCount + m_explicitTermCount + m_privateTermCount << std::endl;
            output << "    Adhoc: " << m_adhocTermCount << std::endl;
            output << "    Explicit: " << m_explicitTermCount << std::endl;
            output << "    Private: " << m_privateTermCount << std::endl;

            output << std::endl;

            output << "  Rows" << std::endl;
            output << "    Total: " << GetAdhocRowCount() + m_currentRow << std::endl;
            output << "    Adhoc: " << GetAdhocRowCount() << std::endl;
            output << "    Explicit: " << m_bins.size() << std::endl;
            output << "    Private: " << m_privateRowCount << std::endl;

            output << std::endl;

            output << "  Bins" << std::endl;
            Accumulator a;
            for (auto bin : m_bins)
            {
                output << "    " << bin.GetFrequency(m_density) << ", " << bin.GetIndex() << std::endl;
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
