#include "IndexUtils.h"
#include "EmptyTermTable.h"
#include "Shard.h"

namespace BitFunnel
{
    size_t GetEmptyTermTableBufferSize(DocIndex capacity,
                                       std::vector<RowIndex> const & rowCounts,
                                       IDocumentDataSchema const & schema)
    {
        EmptyTermTable termTable(rowCounts);
        return Shard::InitializeDescriptors(nullptr,
                                            capacity,
                                            schema,
                                            termTable);
    }


    // WARNING: must be called after Terms and Facts are added to termTable
    // in order for rowCount to be correct.
    size_t GetBufferSize(DocIndex capacity,
                         IDocumentDataSchema const & schema,
                         ITermTable const & termTable)
    {
        return Shard::InitializeDescriptors(nullptr,
                                            capacity,
                                            schema,
                                            termTable);
    }
}
