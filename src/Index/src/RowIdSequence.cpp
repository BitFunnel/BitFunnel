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


//#include "BitFunnel/Index/IFactSet.h"  // For FactHandle.
#include "BitFunnel/ITermTable2.h"
#include "BitFunnel/PackedRowIdSequence.h"
#include "BitFunnel/RowId.h"
#include "BitFunnel/RowIdSequence.h"
#include "BitFunnel/Term.h"
#include "LoggerInterfaces/Logging.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // RowIdSequence
    //
    //*************************************************************************
    // TODO: should GetRawHash be GetClassifiedHash or GetGeneralHash?
    RowIdSequence::RowIdSequence(Term const & term, ITermTable2 const & termTable)
      : m_hash(term.GetRawHash()),
        m_termTable(termTable)
    {
        // TODO: Get rid of out parameter for m_termKind. Consider returning an std::pair.
        // TODO: See if there is any way this can run in the initializer so that members
        // can be const.
        // TODO: Eliminate Shard and Tier from RowId.
        const PackedRowIdSequence packed(termTable.GetRows(term));
        Initialize(packed);
    }


    // TODO: Implement this constructor.
    //RowIdSequence::RowIdSequence(FactHandle fact, ITermTable2 const & termTable)
    //  : m_hash(0),
    //    m_termTable(termTable)
    //{
    //    // TODO: Figure out how to eliminate StreamId::Metaword for facts.
    //    // Do we just create c_MetaWordStreamId? Can user's use this StreamId?
    //    const Term term(static_cast<Term::Hash>(fact),
    //                    StreamId::MetaWord,
    //                    static_cast<Term::IdfX10>(0));

    //    const PackedTermInfo info = termTable.GetTermInfo(term, m_termKind);
    //    Initialize(info);
    //}


    RowIdSequence::const_iterator RowIdSequence::begin() const
    {
        return const_iterator(*this, 0ull);
    }


    RowIdSequence::const_iterator RowIdSequence::end() const
    {
        return const_iterator(*this, m_rowIdCount);
    }


    RowId RowIdSequence::GetRow(size_t row) const
    {
        if (row >= m_rowIdCount)
        {
            RecoverableError error("RowIdSequence::GetRow(): row out of range.");
            throw error;
        }

        // TODO: implement handling for adhoc
        // TODO: implement handling for fact
        return m_termTable.GetRowId(m_rowIdStart + row);

        //// No special clause for ITermTable::Disposed as they return an empty
        //// PackedTermInfo which does not support enumeration.
        //if (m_termKind == ITermTable::Adhoc)
        //{
        //    // TODO: can hash variant be created here in order to eliminate third argument.
        //    return m_termTable.GetRowIdAdhoc(m_hash,
        //                                     m_rowIdStart + row,
        //                                     row);
        //}
        //else if (m_termKind == ITermTable::Fact)
        //{
        //    return m_termTable.GetRowIdForFact(m_rowIdStart + row);
        //}
        //else
        //{
        //    return m_termTable.GetRowId(m_rowIdStart + row);
        //}
    }


    void RowIdSequence::Initialize(PackedRowIdSequence const & packed)
    {
        m_rowIdStart = packed.GetStart();
        m_rowIdCount = packed.GetEnd() - packed.GetStart();
    }
}
