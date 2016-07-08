#include "BitFunnel/Row.h"

#include <inttypes.h>
#include <stddef.h>

#include "BitFunnel/Index/DocumentHandle.h"  // For DocIndex.

namespace BitFunnel
{
    // Helper method.
    static unsigned RoundUp(unsigned value, unsigned quanta)
    {
        return ((value + quanta - 1) / quanta) * quanta;
    }


    //*************************************************************************
    //
    // Row
    //
    //*************************************************************************
    Row::Row(uint64_t const * data)
        : m_data(data)
    {
    }


    uint64_t const * Row::GetData() const
    {
        return m_data;
    }


    unsigned Row::GetAlignment()
    {
        return c_byteAlignment;
    }


    uint64_t* Row::Align(void* address)
    {
        return reinterpret_cast<uint64_t*>((reinterpret_cast<size_t>(address) + static_cast<size_t>(c_byteAlignment) - 1) & ~(static_cast<size_t>(c_byteAlignment) - 1));
    }


    unsigned Row::BytesInRow(DocIndex documentCount, Rank rowRank)
    {
        return DocumentsInRank0Row(documentCount) >> (3 + rowRank);
    }


    DocIndex Row::DocumentsInRank0Row(DocIndex documentCount)
    {
        // DESIGN NOTE: Because the matching engine does quadword loads, it
        // is necessary that c_byteAlignment be at least 8 so that
        // DocumentsInRank0Row() correctly pads the row length.
        static const unsigned c_documentsPerByte = 8;
        unsigned rowQuanta = (c_byteAlignment * c_documentsPerByte) << c_maxRankValue;

        return RoundUp(documentCount, rowQuanta);
    }
}
