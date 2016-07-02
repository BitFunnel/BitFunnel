#include "stdafx.h"

#include "BitFunnel/BitFunnelTypes.h"    // For DocId.
#include "BitFunnel/Factories.h"
#include "DocumentDataSchema.h"

namespace BitFunnel
{
    std::unique_ptr<IDocumentDataSchema> Factories::CreateDocumentDataSchema()
    {
        return std::unique_ptr<IDocumentDataSchema>(new DocumentDataSchema());
    }


    static const size_t s_bytesPerDocId = sizeof(DocId);

    DocumentDataSchema::DocumentDataSchema()
        : m_variableSizeBlobCount(0)
    {
    }


    VariableSizeBlobId DocumentDataSchema::RegisterVariableSizeBlob()
    {
        return m_variableSizeBlobCount++;
    }


    FixedSizeBlobId DocumentDataSchema::RegisterFixedSizeBlob(unsigned byteCount)
    {
        const FixedSizeBlobId blobId = static_cast<FixedSizeBlobId>(m_fixedSizeBlobSizes.size());

        m_fixedSizeBlobSizes.push_back(byteCount);

        return blobId;
    }


    unsigned DocumentDataSchema::GetVariableSizeBlobCount() const
    {
        return m_variableSizeBlobCount;
    }


    std::vector<unsigned> const & DocumentDataSchema::GetFixedSizeBlobSizes() const
    {
        return m_fixedSizeBlobSizes;
    }
}