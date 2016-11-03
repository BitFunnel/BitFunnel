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
#include "BitFunnel/Index/ITermTableBuilder.h"  // Base class.
#include "BitFunnel/Index/RowId.h"              // RowIndex, RowId parameter.
#include "BitFunnel/Term.h"                     // Term::Hash template parameter.
#include "BitFunnel/Utilities/Accumulator.h"    // Accumulator member.
#include "Random.h"


namespace BitFunnel
{
    class DocumentFrequencyTable;   // TODO: IDocumentFrequencyTable
    class IFactSet;
    class ITermTreatment;
    class ITermTable;

    class TermTableBuilder : public ITermTableBuilder
    {
    public:
        TermTableBuilder(double density,
                         double adhocFrequency,
                         ITermTreatment const & treatment,
                         IDocumentFrequencyTable const & terms,
                         IFactSet const & facts,
                         ITermTable & termTable);

        virtual void Print(std::ostream& output) const override;

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
        static size_t GetMinAdhocRowCount();

    private:
        ITermTable & m_termTable;

        class RowAssigner;
        std::vector <std::unique_ptr<RowAssigner>> m_rowAssigners;

        double m_buildTime;

        // Random number generator.
        std::unique_ptr<RandomInt<unsigned>> m_random;

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
                        ITermTable & termTable,
                        RandomInt<unsigned>& random);

            void AssignExplicit(double frequency, RowIndex count, bool isPrivate);
            void AssignAdhoc(double frequency, RowIndex count);

            RowIndex GetExplicitRowCount() const;
            RowIndex GetAdhocRowCount() const;

            void Print(std::ostream& output) const;

        private:
            // Constructor parameters.
            Rank m_rank;
            double m_density;
            ITermTable & m_termTable;

            // Sum of frequencies of all adhoc terms. Used to compute the
            // number of adhoc rows.
            double m_adhocTotal;

            RowIndex m_currentRow;

            class Bin;
            std::set<Bin> m_bins;

            // If any termCount is > 0, then we consider this rank "in use" and
            // set the row count to be at least some minimum value..
            //
            // WARNING: if additional counts are added, GetAdhocRowCount must be
            // updated.
            size_t m_privateExplicitTermCount;
            //     m_privateAdHoc doesn't exist.
            size_t m_sharedAdhocTermCount;
            size_t m_sharedExplicitTermCount;

            size_t m_privateExplicitRowCount;

            RandomInt<unsigned>& m_random;


            class Bin
            {
            public:
                Bin(double density, double frequency, RowIndex index)
                    : m_availableSpace(density - frequency),
                      m_index(index)
                {
                }

                Bin(double frequency)
                    : m_availableSpace(frequency)
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
                    m_availableSpace -= frequency;
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
                double m_availableSpace;
                RowIndex m_index;
            };
        };
    };
}
