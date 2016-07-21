#include <vector>

#include "BitFunnel/BitFunnelTypes.h"  // For DocIndex.
#include "BitFunnel/RowId.h"  // For RowIndex.

namespace BitFunnel
{
    class IDocumentDataSchema;
    class ITermTable;

    // Given a slice capacity, get the necessary buffer size to accomodate that
    // capacity.
    //
    // WARNING: must be called after Terms and Facts are added to termTable in
    // order for rowCount to be correct.
    size_t GetBufferSize(DocIndex capacity,
                         IDocumentDataSchema const & schema,
                         ITermTable const & termTable);

    size_t GetEmptyTermTableBufferSize(DocIndex capacity,
                                       std::vector<RowIndex> const & rowCounts,
                                       IDocumentDataSchema const & schema);
}
