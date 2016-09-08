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

#include "BitFunnel/AbstractRow.h"
#include "BitFunnel/IObjectFormatter.h"
#include "BitFunnel/IObjectParser.h"
#include "LoggerInterfaces/Logging.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // AbstractRow
    //
    //*************************************************************************
    AbstractRow::AbstractRow(unsigned id, Rank rank, bool inverted)
        : m_id(static_cast<uint16_t>(id)),
          m_rank(static_cast<uint16_t>(rank)),
          m_rankDelta(0),
          m_inverted(inverted)
    {
        LogAssertB(id <= 0xffff, "id overflow");
    }


    AbstractRow::AbstractRow(AbstractRow const & row, Rank rankDelta)
        : m_id(row.m_id),
          m_rank(static_cast<uint16_t>(row.m_rank + row.m_rankDelta - rankDelta)),
          m_rankDelta(static_cast<uint16_t>(rankDelta)),
          m_inverted(row.m_inverted)
    {
        LogAssertB(rankDelta <= static_cast<Rank>(row.m_rank + row.m_rankDelta),
                   "rankDelta overflow");
    }


    static const char* c_adhocRowPrimitiveName = "Row";


    AbstractRow::AbstractRow(IObjectParser& parser, bool parseParametersOnly)
    {
        if (!parseParametersOnly)
        {
            parser.OpenPrimitive(c_adhocRowPrimitiveName);
        }

        LogAssertB(parser.OpenPrimitiveItem(),
                   "AbstractRow failed to OpenPrimitiveItem");
        unsigned id = parser.ParseUInt();
        LogAssertB(id <= 0xffff,
                   "id overflow");
        m_id = static_cast<uint16_t>(id);

        LogAssertB(parser.OpenPrimitiveItem(),
                   "AbstractRow failed to OpenPrimitiveItem");
        m_rank = static_cast<uint16_t>(parser.ParseUInt());
        LogAssertB(m_rank <= c_maxRankValue, "rank > c_maxRankValue");

        LogAssertB(parser.OpenPrimitiveItem(),
                   "AbstractRow failed to OpenPrimitiveItem");
        m_rankDelta = static_cast<uint16_t>(parser.ParseUInt());
        LogAssertB(m_rankDelta + m_rank <= c_maxRankValue,
                   "rank delta overflow");

        LogAssertB(parser.OpenPrimitiveItem(),
                   "AbstractRow failed to OpenPrimitiveItem");
        m_inverted = parser.ParseBool();

        if (!parseParametersOnly)
        {
            parser.ClosePrimitive();
        }
    }


    void AbstractRow::Format(IObjectFormatter& formatter,
                             const char* name) const
    {
        if (name == nullptr)
        {
            name = c_adhocRowPrimitiveName;
        }

        formatter.OpenPrimitive(name);

        formatter.OpenPrimitiveItem();
        formatter.Format(static_cast<unsigned>(m_id));

        formatter.OpenPrimitiveItem();
        formatter.Format(static_cast<unsigned>(m_rank));

        formatter.OpenPrimitiveItem();
        formatter.Format(static_cast<unsigned>(m_rankDelta));

        formatter.OpenPrimitiveItem();
        formatter.Format(m_inverted);

        formatter.ClosePrimitive();
    }


    unsigned AbstractRow::GetId() const
    {
        return m_id;
    }


    Rank AbstractRow::GetRank() const
    {
        return m_rank;
    }


    Rank AbstractRow::GetRankDelta() const
    {
        return m_rankDelta;
    }


    bool AbstractRow::IsInverted() const
    {
        return m_inverted;
    }
}
