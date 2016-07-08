#include <stdexcept>

#include "BitFunnel/PackedTermInfo.h"
#include "BitFunnel/Row.h"  // For c_maxRankValue.
#include "BitFunnel/RowId.h"
#include "EmptyTermTable.h"
#include "LoggerInterfaces/Logging.h"
#include "Term.h"


// TODO: move this somewhere appropriate and make it a real value.
#define c_systemRowCount 3

namespace BitFunnel
{
    EmptyTermTable::EmptyTermTable()
        : m_rowCounts(c_maxRankValue + 1, c_systemRowCount)
    {
        LogAssertB(static_cast<size_t>(m_rowCounts.size()) ==
                   (c_maxRankValue + 1),
                   "");
    }


    EmptyTermTable::EmptyTermTable(std::vector<size_t> const & rowCounts)
        : m_rowCounts(rowCounts)
    {
        LogAssertB(static_cast<size_t>(rowCounts.size()) ==
                   (c_maxRankValue + 1),
                   "");
    }


    size_t EmptyTermTable::GetTotalRowCount(size_t rank) const
    {
        return m_rowCounts[rank];
    }


    void EmptyTermTable::Write(std::ostream& /* stream */) const
    {
        throw std::runtime_error("Not implemented");
    }


    void EmptyTermTable::AddRowId(RowId /* id */)
    {
        throw std::runtime_error("Not implemented");
    }


    unsigned EmptyTermTable::GetRowIdCount() const
    {
        throw std::runtime_error("Not implemented");
    }


    RowId EmptyTermTable::GetRowId(unsigned /* rowOffset */) const
    {
        throw std::runtime_error("Not implemented");
    }


    RowId EmptyTermTable::GetRowIdAdhoc(uint64_t /* hash */,
                                        unsigned /* rowOffset */,
                                        unsigned /* variant */)  const
    {
        throw std::runtime_error("Not implemented");
    }


    RowId EmptyTermTable::GetRowIdForFact(unsigned rowOffset) const
    {
        LogAssertB(m_rowCounts[0] >= c_systemRowCount,
                   "Rank 0 must contain at least c_systemRowCount rows when calling GetRowIdForFact()");

        // EmptyTermTable assumes rows for facts go first.
        LogAssertB(rowOffset < c_systemRowCount,
                   "rowOffset too large.");
        return RowId(0, 0, rowOffset);
    }


    void EmptyTermTable::AddTerm(uint64_t /* hash */,
                                 unsigned /* rowIdOffset */,
                                 unsigned /* rowIdLength */)
    {
        throw std::runtime_error("Not implemented");
    }


    // void EmptyTermTable::AddTermAdhoc(Stream::Classification /* classification */,
    //                                   unsigned /* gramSize */,
    //                                   Tier /* tierHint */,
    //                                   IdfSumX10 /* idfSum */,
    //                                   unsigned /* rowIdOffset */,
    //                                   unsigned /* rowIdLength */)
    // {
    //     throw std::runtime_error("Not implemented");
    // }


    void EmptyTermTable::SetRowTableSize(size_t /* rank */,
                                         unsigned /* rowCount */,
                                         unsigned /* sharedRowCount */)
    {
        throw std::runtime_error("Not implemented");
    }


    size_t EmptyTermTable::GetMutableFactRowCount(size_t /* rank */) const
    {
        throw std::runtime_error("Not implemented");
    }


    PackedTermInfo EmptyTermTable::GetTermInfo(const Term& term,
                                               TermKind& termKind) const
    {
        // Soft-deleted row.
        if (static_cast<unsigned>(term.GetRawHash()) < c_systemRowCount)
        {
            LogAssertB(m_rowCounts[0] >= c_systemRowCount,
                       "Rank 0 must contain at least c_systemRowCount rows when calling GetTermInfo()");
            termKind = TermKind::Fact;
            return PackedTermInfo(static_cast<unsigned>(term.GetRawHash()), 1);
        }
        else
        {
            throw std::runtime_error("Not implemented");
        }
    }
}
