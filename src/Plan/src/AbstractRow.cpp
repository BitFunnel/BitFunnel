#include "stdafx.h"

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
        : m_id(static_cast<unsigned __int16>(id)),
          m_rank(static_cast<unsigned __int16>(rank)),
          m_rankDelta(0),
          m_inverted(inverted)
    {
        LogAssertB(id <= 0xffff);
    }


    AbstractRow::AbstractRow(AbstractRow const & row, Rank rankDelta)
        : m_id(row.m_id),
          m_rank(static_cast<unsigned __int16>(row.m_rank + row.m_rankDelta - rankDelta)),
          m_rankDelta(static_cast<unsigned __int16>(rankDelta)),
          m_inverted(row.m_inverted)
    {
        LogAssertB(rankDelta <= static_cast<Rank>(row.m_rank + row.m_rankDelta));
    }


    static const char* c_adhocRowPrimitiveName = "Row";


    AbstractRow::AbstractRow(IObjectParser& parser, bool parseParametersOnly)
    {
        if (!parseParametersOnly)
        {
            parser.OpenPrimitive(c_adhocRowPrimitiveName);
        }

        LogAssertB(parser.OpenPrimitiveItem());
        unsigned id = parser.ParseUInt();
        LogAssertB(id <= 0xffff);
        m_id = static_cast<unsigned __int16>(id);

        LogAssertB(parser.OpenPrimitiveItem());
        m_rank = static_cast<unsigned __int16>(parser.ParseUInt());
        LogAssertB(m_rank <= c_maxRankValue);

        LogAssertB(parser.OpenPrimitiveItem());
        m_rankDelta = static_cast<unsigned __int16>(parser.ParseUInt());
        LogAssertB(m_rankDelta + m_rank <= c_maxRankValue);

        LogAssertB(parser.OpenPrimitiveItem());
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
