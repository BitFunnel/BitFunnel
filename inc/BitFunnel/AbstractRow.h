#pragma once

#include "BitFunnel/BitFunnelTypes.h"


namespace BitFunnel
{
    class IObjectFormatter;
    class IObjectParser;

    //*************************************************************************
    //
    // AbstractRow represents a row in a RowPlan.
    //
    // AbstractRow differs from the more concrete RowId in that it doesn't
    // specify a shard and its row index specifies an entry into a table of
    // RowIds used in the plan instead the actual row positition in a RowTable.
    //
    // AbstractRows also introduce the concept of a RankDelta, which is used to
    // specify the difference between the row's rank and the rank at which it
    // will be evaluated. When rows can be arranged by descending rank, all
    // RankDelta values will be zero and the matching process will bifrucate or
    // "rank down" for each new rank value. In some cases the ranks cannot be
    // placed in strictly decreasing order. When this happens, the RankDelta
    // value provides information to compute the correct offset into the row
    // with the higher rank.
    //
    //*************************************************************************
    class AbstractRow
    {
    public:
        // Constructs an abstract row with specified id, rank and inverted
        // state. RankDelta is set to zero.
        AbstractRow(unsigned id, Rank rank, bool inverted);

        // Constructs an abstract row with a specified rankDelta, using the
        // id, rank, and inverted status provided by the row parameter.
        AbstractRow(AbstractRow const & row, Rank rankDelta);

        // Constructs an abstract row from an IObjectParser.
        AbstractRow(IObjectParser& parser, bool parseParametersOnly);

        // Serializes an the row to an IObjectFormatter. The name parameter
        // provides the text identifier for the row in text serialization
        // formats.
        void Format(IObjectFormatter& formatter, const char* name) const;


        // Returns the row's id. This is the row's number in the RowPlan and
        // is used to look up the corresponding RowId in a table.
        unsigned GetId() const;


        // Returns the rank at which the row will be evaluated. Note that the
        // row's actual rank is the sum of GetRank() and GetRankDelta().
        Rank GetRank() const;

        // Returns the row's rank delta. The rank delta is the difference
        // between the row's rank and the rank at which it will be evaluated
        // in the plan.
        Rank GetRankDelta() const;

        // Returns true if the row is inverted in the plan. Inverted rows
        // correspond to boolean negation.
        bool IsInverted() const;

    private:
        uint16_t m_id;
        uint16_t m_rank;
        uint16_t m_rankDelta;
        bool m_inverted;
    };
}
