#pragma once

#include "BitFunnel/BitFunnelTypes.h"         // c_maxRankValue used in declaration.
#include "BitFunnel/IEnumerator.h"            // Inherits from IEnumerator.
#include "BitFunnel/Index/IFactSet.h"         // Embeds FactHandle.
#include "BitFunnel/Plan/IPlanRows.h"         // AbstractRow as template parameter.
#include "BitFunnel/NonCopyable.h"            // Inherits from NonCopyable.
#include "FixedCapacityVector.h"              // FixedCapacityVector embedded.


namespace BitFunnel
{
    class IPlanRows;
    class Term;
    class RowIdSequence;

    //*************************************************************************
    //
    // AbstractRowEnumerator performs the following three functions:
    //   1. Updates an IPlanRows with the AbstractRows for a specific Term or
    //      Fact passed to the constructor.
    //   2. Updates the IPlanRow's mapping from AbstractRow index to RowId.
    //   3. Provides an IEnumerator for the AbstractRows associated with the
    //      Term or Fact passed to the constructor.
    //
    // TYPICAL USAGE
    // The class is intended to be used during the conversion from Term-level
    // plans to Row-level plans. At the beginning of the conversion, an empty
    // IPlanRows is created. The IPlanRows will hold the mapping from the
    // AbstractRow ids used in the generic, cross-shard plan to the physical
    // rows required to execute the plan in a specific shard. As each Term or
    // Fact is encountered during the conversion, an AbstractRowEnumerator is
    // created. The constructor of the AbstractRowEnumerator looks up the rows
    // associated with the Term/Fact in each Shard's TermTable and then adds
    // the appropriate entries to the IPlanRows. Once the AbstractRowEnumerator
    // has been constructed, it can be used to enumerate the resulting
    // AbstractRows which will be incorporated into the Row-level plan.
    //
    //*************************************************************************
    class AbstractRowEnumerator : public IEnumerator<AbstractRow>, NonCopyable
    {
    public:
        // Looks up the RowIds associated with the Term in each of the Shards,
        // determines the number of AbstractRows required, then constructs the
        // mapping from AbstractRow to RowId in the IPlanRows.
        AbstractRowEnumerator(const Term& term, IPlanRows& planRows);

        // TODO: implement this constructor.
        // Looks up the RowIds associated with the Fact in each of the Shards,
        // determines the number of AbstractRows required, then constructs the
        // mapping from AbstractRow to RowId in the IPlanRows.
        // AbstractRowEnumerator(const FactHandle& fact, IPlanRows& planRows);

        //
        // IEnumerator<AbstractRow*> methods.
        //

        // Enumerates the AbstractRows in the mapping in descending Rank order.
        bool MoveNext();
        AbstractRow Current() const;
        void Reset();

    private:

        // Get the Ids of system rows which will be used for plan row generation.
        void GetSystemRowIds(IPlanRows& planRows);

        // Called by the constructor. Uses RowIdSequence to looks up the RowIds
        // for a term of a fact in a single Shard and adds them to the
        // IPlanRows.
        void ProcessShard(RowIdSequence& rowIds,
                          IPlanRows& planRows,
                          ShardId shard);

        // Called by the constructor after looking up RowIds for m_term or
        // m_fact. Prepares the enumerator for its first use.
        void FinishInitialization(IPlanRows& planRows);

        // Called by the constructor after all RowIds for all shards have been
        // added to the IPlanRows. On entry, number of RowIds at a given Rank
        // may vary across the Shards. This method duplicates RowIds in some
        // Shards to ensure that each Rank has the same number of RowIds across
        // all Shards. This is essential in creating a single RowPlan that can
        // be run on all Shards.
        void PadWithDuplicateRowIds(IPlanRows& planRows);

        // For enumerators created for terms, this holds a pointer to the term.
        // Used for logging.
        const Term * const m_term;

        // For enumerators created for facts, this holds a pointer to the fact.
        // Used for logging.
        // const FactHandle * const m_fact;

        // Storage for the AbstractRows created by the IPlanRows during
        // AbstractRowEnumerator construction. These AbstractRows are the
        // backing store for the IEnumerator.
        FixedCapacityVector<AbstractRow, c_maxRowsPerTerm> m_rows[c_maxRankValue + 1];

        // Current rank in the enumeration. Ranks are enumerated in descending
        // order. DESIGN NOTE: using int instead of unsigned to allow while
        // loop with descending counter that halts after zero.
        int m_currentRank;

        // Current row in the enumeration. DESIGN NOTE: using int instead of
        // unsigned to allow -1 to be used as a sentinal for Reset().
        int m_currentRow;

        // The list of RowIds for match-all and match-none term in all the shards.
        RowId m_matchAllTermRowIds[c_maxShardIdCount];
        RowId m_matchNoneTermRowIds[c_maxShardIdCount];
    };
}
