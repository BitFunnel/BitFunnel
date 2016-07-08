// TODO: figure out correct location for this file.

#include "BitFunnel/PackedTermInfo.h"
#include "BitFunnel/RowId.h"
#include "BitFunnel/Term.h"
#include "BitFunnel/TermInfo.h"
#include "LoggerInterfaces/Logging.h"


namespace BitFunnel
{
    // TODO: should GetRawHash be GetClassifiedHash or GetGeneralHash?
    TermInfo::TermInfo(Term const & term, ITermTable const & termTable)
        : m_hash(term.GetRawHash()),
          m_termTable(termTable)
    {
        const PackedTermInfo info = termTable.GetTermInfo(term, m_termKind);
        Initialize(info);
    }


    // TermInfo::TermInfo(FactHandle fact, ITermTable const & termTable)
    //     : m_hash(0),
    //       m_termTable(termTable)
    // {
    //     const Term term(static_cast<Term::Hash>(fact),
    //                     Stream::MetaWord,
    //                     static_cast<IdfX10>(0));

    //     const PackedTermInfo info = termTable.GetTermInfo(term, m_termKind);
    //     Initialize(info);
    // }


    bool TermInfo::IsEmpty() const
    {
        return (m_rowIdCount == 0);
    }


    bool TermInfo::MoveNext()
    {
        LogAssertB(m_currentRow < static_cast<int>(m_rowIdCount),
                   "MoveNext overflows row range.");
        return (++m_currentRow < static_cast<int>(m_rowIdCount));
    }


    void TermInfo::Reset()
    {
        m_currentRow = -1;
    }


    RowId TermInfo::Current() const
    {
        LogAssertB(m_currentRow >= 0 &&
                   m_currentRow < static_cast<int>(m_rowIdCount),
                   "m_currentRow out of range.");

        // No special clause for ITermTable::Disposed as they return an empty
        // PackedTermInfo which does not support enumeration.
        if (m_termKind == ITermTable::Adhoc)
        {
            return m_termTable.GetRowIdAdhoc(m_hash,
                                             m_rowIdStart + m_currentRow,
                                             static_cast<unsigned>(m_currentRow));
        }
        else if (m_termKind == ITermTable::Fact)
        {
            return m_termTable.GetRowIdForFact(m_rowIdStart + m_currentRow);
        }
        else
        {
            return m_termTable.GetRowId(m_rowIdStart
                                        + static_cast<unsigned>(m_currentRow));
        }
    }


    void TermInfo::Initialize(PackedTermInfo const & info)
    {
        m_rowIdStart = info.GetRowIdStart();
        m_rowIdCount = info.GetRowIdCount();

        Reset();
    }
}
