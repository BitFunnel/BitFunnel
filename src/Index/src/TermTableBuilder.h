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

#include <iosfwd>                               // std::ostream parameter.
#include <iterator>                             // typedef uses std::back_inserter_iterator.
#include <map>                                  // std::map member.
#include <memory>                               // std::unique_ptr member.
#include <set>                                  // std::set member.
#include <vector>                               // std::vector member.

#include "BitFunnel/BitFunnelTypes.h"           // Rank parameter.
#include "BitFunnel/RowId.h"                    // RowIndex, RowId parameter.
#include "BitFunnel/Term.h"                     // Term::Hash template parameter.
#include "BitFunnel/Utilities/Accumulator.h"    // Accumulator member.


namespace BitFunnel
{
    class DocumentFrequencyTable;   // TODO: IDocumentFrequencyTable
    class ITermTreatment;

    class TermTableBuilder
    {
    public:
        TermTableBuilder(double density,
                         double adhocFrequency,
                         ITermTreatment const & treatment,
                         DocumentFrequencyTable const & terms);

        void Print(std::ostream& output) const;

    private:
        std::vector<RowId> m_rowAssignments;
        typedef std::back_insert_iterator<decltype(m_rowAssignments)> RowIdInserter;

        class RowAssignment;
        std::map<Term::Hash, RowAssignment> m_map;

        class RowAssigner;
        std::vector <std::unique_ptr<RowAssigner>> m_rowAssigners;

        class RowAssignment
        {
        public:
            RowAssignment(size_t start, size_t end)
                : m_start(static_cast<unsigned>(start)),
                  m_end(static_cast<unsigned>(end))
            {
            }

            unsigned GetStart() const
            {
                return m_start;
            }

            unsigned GetEnd() const
            {
                return m_end;
            }

        private:
            unsigned m_start;
            unsigned m_end;
        };


        class RowAssigner
        {
        public:
            RowAssigner(Rank rank,
                        double density,
                        double adhocFrequency,
                        RowIdInserter inserter);

            void Assign(double frequency, RowIndex count, bool isPrivate);

            RowIndex GetExplicitRowCount() const;
            RowIndex GetAdhocRowCount() const;

            void Print(std::ostream& output) const;

        private:
            // Constructor parameters.
            Rank m_rank;
            double m_density;
            double m_adhocFrequency;
            TermTableBuilder::RowIdInserter& m_inserter;

            // Sum of frequencies of all adhoc terms. Used to compute the
            // number of adhoc rows.
            double m_adhocTotal;

            RowIndex m_currentRow;

            class Bin;
            std::set<Bin> m_bins;

            size_t m_explicitTermCount;
            size_t m_adhocTermCount;
            size_t m_privateTermCount;
            size_t m_privateRowCount;

            class Bin
            {
            public:
                Bin(double density, double frequency, RowIndex index)
                    : m_availableSpace(static_cast<float>(density - frequency)),
                      m_index(index)
                {
                }

                Bin(double frequency)
                    : m_availableSpace(static_cast<float>(frequency))
                {
                }

                double GetAvailableSpace() const
                {
                    return m_availableSpace;
                }

                double GetFrequency(double density) const
                {
                    return density - m_availableSpace;
                }

                RowIndex GetIndex() const
                {
                    return m_index;
                }

                void Reserve(double frequency)
                {
                    // TODO: check for overflow.
                    m_availableSpace -= static_cast<float>(frequency);
                }

                bool operator<(Bin const & other) const
                {
                    if (m_availableSpace != other.m_availableSpace)
                    {
                        return m_availableSpace < other.m_availableSpace;
                    }
                    else
                    {
                        return m_index < other.m_index;
                    }
                }

            private:
                float m_availableSpace;
                RowIndex m_index;
            };
        };
    };
}
