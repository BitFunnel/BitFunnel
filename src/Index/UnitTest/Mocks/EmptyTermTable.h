#pragma once

#include <vector>

#include "BitFunnel/BitFunnelTypes.h"
#include "BitFunnel/ITermTable.h"
#include "BitFunnel/NonCopyable.h"

namespace BitFunnel
{
    class EmptyTermTable : public ITermTable, NonCopyable
    {
    public:
        EmptyTermTable();
        EmptyTermTable(std::vector<RowIndex> const & rowCounts);      

        virtual RowIndex GetTotalRowCount(Tier, Rank) const override;

        // These methods are not applicable for the tests and throw if called.
        virtual void Write(std::ostream& stream) const override;
        virtual void AddRowId(RowId id) override;
        virtual unsigned GetRowIdCount() const override;
        virtual RowId GetRowId(unsigned rowOffset) const override;
        virtual RowId GetRowIdAdhoc(Term::Hash hash, unsigned rowOffset, unsigned variant)  const override;
        virtual RowId GetRowIdForFact(unsigned rowOffset) const override;
        virtual void AddTerm(Term::Hash hash, unsigned rowIdOffset, unsigned rowIdLength) override;
        virtual void AddTermAdhoc(Stream::Classification classification,
                                  unsigned gramSize,
                                  Tier tierHint,
                                  IdfSumX10 idfSum,
                                  unsigned rowIdOffset,
                                  unsigned rowIdLength) override;
        virtual void SetRowTableSize(Tier tier,
                                     Rank rank,
                                     unsigned rowCount,
                                     unsigned sharedRowCount) override;
        virtual RowIndex GetMutableFactRowCount(Tier tier, Rank rank) const override;
        virtual PackedTermInfo GetTermInfo(const Term& term, TermKind& termKind) const override;

    private:
        const std::vector<RowIndex> m_rowCounts;

    };
}