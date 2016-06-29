#include "stdafx.h"

#include "BitFunnel/PackedTermInfo.h"
#include "EmptyTermTable.h"
#include "LoggerInterfaces/Logging.h"


namespace BitFunnel
{
    EmptyTermTable::EmptyTermTable()
        : m_rowCounts(c_maxRankValue + 1, c_systemRowCount)
    {
        LogAssertB(static_cast<Rank>(m_rowCounts.size()) == (c_maxRankValue + 1));
    }


    EmptyTermTable::EmptyTermTable(std::vector<RowIndex> const & rowCounts)
        : m_rowCounts(rowCounts)
    {
        LogAssertB(static_cast<Rank>(rowCounts.size()) == (c_maxRankValue + 1));
    }    


    RowIndex EmptyTermTable::GetTotalRowCount(Tier, Rank rank) const
    {
        return m_rowCounts[rank];
    }


    void EmptyTermTable::Write(std::ostream& /* stream */) const
    {
        throw std::exception("Not implemented");
    }


    void EmptyTermTable::AddRowId(RowId /* id */)
    {
        throw std::exception("Not implemented");
    }


    unsigned EmptyTermTable::GetRowIdCount() const
    {
        throw std::exception("Not implemented");
    }


    RowId EmptyTermTable::GetRowId(unsigned /* rowOffset */) const
    {
        throw std::exception("Not implemented");
    }


    RowId EmptyTermTable::GetRowIdAdhoc(Term::Hash /* hash */, 
                                        unsigned /* rowOffset */, 
                                        unsigned /* variant */)  const
    {
        throw std::exception("Not implemented");
    }


    RowId EmptyTermTable::GetRowIdForFact(unsigned rowOffset) const
    {
        LogAssertB(m_rowCounts[0] >= c_systemRowCount,
                   "Rank 0 must contain at least c_systemRowCount rows when calling GetRowIdForFact()");

        // EmptyTermTable assumes rows for facts go first.
        LogAssertB(rowOffset < c_systemRowCount);
        return RowId(0, DDRTier, 0, rowOffset);
    }


    void EmptyTermTable::AddTerm(Term::Hash /* hash */, 
                                 unsigned /* rowIdOffset */, 
                                 unsigned /* rowIdLength */)
    {
        throw std::exception("Not implemented");
    }


    void EmptyTermTable::AddTermAdhoc(Stream::Classification /* classification */,
                                      unsigned /* gramSize */,
                                      Tier /* tierHint */,
                                      IdfSumX10 /* idfSum */,
                                      unsigned /* rowIdOffset */,
                                      unsigned /* rowIdLength */)
    {
        throw std::exception("Not implemented");
    }


    void EmptyTermTable::SetRowTableSize(Tier /* tier */,
                                         Rank /* rank */,
                                         unsigned /* rowCount */,
                                         unsigned /* sharedRowCount */)
    {
        throw std::exception("Not implemented");
    }


    RowIndex EmptyTermTable::GetMutableFactRowCount(Tier /* tier */, 
                                                    Rank /* rank */) const
    {
        throw std::exception("Not implemented");
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
            throw std::exception("Not implemented");
        }
    }
}