#pragma once

#include <vector>

#include "BitFunnel/ITermTable.h"
#include "BitFunnel/NonCopyable.h"
#include "BitFunnel/RowId.h"

namespace BitFunnel
{
    class EmptyTermTable : public ITermTable, NonCopyable
    {
    public:
        EmptyTermTable();
        EmptyTermTable(std::vector<RowIndex> const & rowCounts);

        virtual size_t GetTotalRowCount(size_t) const override;

        // These methods are not applicable for the tests and throw if called.
        virtual void Write(std::ostream& stream) const override;
        virtual void AddRowId(RowId id) override;
        virtual size_t GetRowIdCount() const override;
        virtual RowId GetRowId(size_t rowOffset) const override;
        virtual RowId GetRowIdAdhoc(uint64_t hash, size_t rowOffset, size_t variant)  const override;
        virtual RowId GetRowIdForFact(size_t rowOffset) const override;
        virtual void AddTerm(uint64_t hash, size_t rowIdOffset, size_t rowIdLength) override;
        /*
        virtual void AddTermAdhoc(Stream::Classification classification,
                                  unsigned gramSize,
                                  Tier tierHint,
                                  IdfSumX10 idfSum,
                                  unsigned rowIdOffset,
                                  unsigned rowIdLength) override;
        */
        virtual void SetRowTableSize(size_t rank,
                                     size_t rowCount,
                                     size_t sharedRowCount) override;
        virtual size_t GetMutableFactRowCount(size_t rank) const override;
        virtual PackedTermInfo GetTermInfo(const Term& term, TermKind& termKind) const override;

    private:
        const std::vector<RowIndex> m_rowCounts;

    };
}
