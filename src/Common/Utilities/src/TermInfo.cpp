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


// TODO: figure out correct location for this file.
// TODO: for that matter, figure out correction location for everything that
// used to be in Core.

#include "BitFunnel/Index/IFactSet.h"  // For FactHandle.
#include "BitFunnel/PackedTermInfo.h"
#include "BitFunnel/RowId.h"
#include "BitFunnel/Stream.h"  // For StreamId.
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


    TermInfo::TermInfo(FactHandle fact, ITermTable const & termTable)
        : m_hash(0),
          m_termTable(termTable)
    {
        const Term term(static_cast<Term::Hash>(fact),
                        StreamId::MetaWord,
                        static_cast<Term::IdfX10>(0));

        const PackedTermInfo info = termTable.GetTermInfo(term, m_termKind);
        Initialize(info);
    }


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
            return m_termTable.
                GetRowIdAdhoc(m_hash,
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
