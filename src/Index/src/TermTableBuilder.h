#pragma once

#include <set>                          // std::set member.

#include "BitFunnel/BitFunnelTypes.h"   // Rank parameter.
#include "BitFunnel/RowId.h"            // RowIndex parameter.


namespace BitFunnel
{
    class DocumentFrequencyTable;   // TODO: IDocumentFrequencyTable
    class ITermTreatment;

    class TermTableBuilder
    {
    public:
        TermTableBuilder(ITermTreatment const & treatment,
                         DocumentFrequencyTable const & terms);

    };


    class RowAssigner
    {
    public:
        RowAssigner(Rank rank, double density, double adhocFrequency, TermTable::iterator& it);

        void Assign(double frequency, RowIndex count);

    private:
        class Bin
        {
        public:
            Bin(double density, double frequency, RowIndex index)
                : m_space(density - frequency),
                  m_index(index)
            {
            }

            double GetAvailableSpace() const
            {
                return m_space;
            }

            double GetFrequency(double density) const
            {
                return density - m_space;
            }

            RowIndex GetIndex() const
            {
                return m_index;
            }

            void Reserve(double frequency)
            {
                // TODO: check for overflow.
                m_space -= frequency;
            }

            bool operator<(Bin const & other) const;

        private:
            float m_availableSpace;
            RowIndex m_index;
        };

        std::set<Bin> m_bins;
    };
}
