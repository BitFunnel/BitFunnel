#pragma once

#include <vector>

#include "BitFunnel/ITermTable.h"
#include "BitFunnel/NonCopyable.h"

namespace BitFunnel
{
    class RowId;

    class EmptyTermTable : public ITermTable, NonCopyable
    {
    public:
        EmptyTermTable();
        EmptyTermTable(std::vector<size_t> const & rowCounts);

        virtual size_t GetTotalRowCount(size_t) const override;

        // These methods are not applicable for the tests and throw if called.
        virtual void Write(std::ostream& stream) const override;
        virtual void AddRowId(RowId id) override;
        virtual unsigned GetRowIdCount() const override;
        virtual RowId GetRowId(unsigned rowOffset) const override;
        virtual RowId GetRowIdAdhoc(uint64_t hash, unsigned rowOffset, unsigned variant)  const override;
        virtual RowId GetRowIdForFact(unsigned rowOffset) const override;
        virtual void AddTerm(uint64_t hash, unsigned rowIdOffset, unsigned rowIdLength) override;
        /*
        virtual void AddTermAdhoc(Stream::Classification classification,
                                  unsigned gramSize,
                                  Tier tierHint,
                                  IdfSumX10 idfSum,
                                  unsigned rowIdOffset,
                                  unsigned rowIdLength) override;
        */
        virtual void SetRowTableSize(size_t rank,
                                     unsigned rowCount,
                                     unsigned sharedRowCount) override;
        virtual size_t GetMutableFactRowCount(size_t rank) const override;
        virtual PackedTermInfo GetTermInfo(const Term& term, TermKind& termKind) const override;

    private:
        const std::vector<size_t> m_rowCounts;

    };
}
