#include "LoggerInterfaces/Logging.h"
#include "BitFunnel/PackedTermInfo.h"


namespace BitFunnel
{
    PackedTermInfo::PackedTermInfo(unsigned rowIdStart, unsigned rowIdCount)
        : m_rowIdStart(rowIdStart),
          m_rowIdCount(static_cast<const uint8_t>(rowIdCount))
    {
        // Check for overflow in cast to uint8_t.
        LogAssertB(m_rowIdCount == rowIdCount, "overflow in cast to uint8_t");
    }


    PackedTermInfo::PackedTermInfo()
        : m_rowIdStart(0),
          m_rowIdCount(0)
    {
    }


    bool PackedTermInfo::IsEmpty() const
    {
        return (m_rowIdCount == 0);
    }


    unsigned PackedTermInfo::GetRowIdStart() const
    {
        return m_rowIdStart;
    }


    unsigned PackedTermInfo::GetRowIdCount() const
    {
        return m_rowIdCount;
    }


    bool PackedTermInfo::operator==(const PackedTermInfo& rhs) const
    {
        return (m_rowIdStart == rhs.m_rowIdStart)
               && (m_rowIdCount == rhs.m_rowIdCount);
    }
}
