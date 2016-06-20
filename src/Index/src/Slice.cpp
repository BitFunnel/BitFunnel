#include "Slice.h"


namespace BitFunnel
{
    Slice::Slice(Shard& shard)
        : m_shard(shard),
          m_temporaryNextDocIndex(0U)
    {
    }


    Shard& Slice::GetShard() const
    {
        return m_shard;
    }


    bool Slice::TryAllocateDocument(DocIndex& index)
    {
        // TODO: Correct implementation.
        index = m_temporaryNextDocIndex++;
        return true;
    }
}
