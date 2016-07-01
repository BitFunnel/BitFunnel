#pragma once

#include <inttypes.h>  // For uint8_t.
#include <stddef.h>  // For size_t.

// #include "BitFunnel/BitFunnelTypes.h"     // PackedTermInfo embeds RowIndex.
// #include "BitFunnel/RowId.h"              // For RowIndex.

namespace BitFunnel
{
    class Term;

    //*************************************************************************
    //
    // PackedTermInfo
    //
    // PackedTermInfo stores the RowIds associated with a Term in the context
    // of some TermTable. The RowIds are represented as a contiguous sequence
    // of slots in the TermTable's RowId buffer.
    //
    // The TermInfo class decodes the PackedTermInfo in the context of a
    // particular TermTable.
    //
    // DESIGN NOTE: PackedTermInfo is a POD type (plain old data) and is
    // intended to be passed and returned as a value type.
    //
    //*************************************************************************
    // TODO: packing.
    //#pragma pack(push, 1)
      // maximally compact object, optimize size over performance.
    //    __declspec (align(1))
    class PackedTermInfo
    {
    public:
        // Constructs a PackedTermInfo initialized with provided values.
        PackedTermInfo(unsigned rowIdStart, unsigned rowIdCount);

        // Constructs an empty PackTermInfo() with m_rowIdCount == 0.
        // Mainly used for creating SimpleHashTable<PackedTermInfo> in TermTable.
        PackedTermInfo();

        // Returns true if m_rowIdCount == 0.
        bool IsEmpty() const;

        // Returns the offset of the first row in the TermTable's RowId table.
        unsigned GetRowIdStart() const;

        // Returns the number of consecutive rows in the TermTable's RowId table.
        unsigned GetRowIdCount() const;

        // Returns true the objects are equal.
        bool operator==(const PackedTermInfo& rhs) const;

    private:
        // Index into the TermTable's RowId table.
        size_t m_rowIdStart;

        // Number of consecutive RowIds
        uint8_t m_rowIdCount;
    };
    // TODO: packing.
    // #pragma pack(pop)
}
