#pragma once

#include <vector>

#include "BitFunnel/Index/IDocumentDataSchema.h"

namespace BitFunnel
{
    //*************************************************************************
    //
    // DocumentDataSchema is an implementation of IDocumentDataSchema which
    // gives out sequential numbers for blob IDs.
    //
    // Thread safety: thread safe for multiple readers. Not thread safe for
    // multiple writers.
    //
    //*************************************************************************
    class DocumentDataSchema : public IDocumentDataSchema
    {
    public:
        DocumentDataSchema();

        //
        // IDocumentDataSchema API.
        //
        virtual VariableSizeBlobId RegisterVariableSizeBlob() override;
        virtual FixedSizeBlobId RegisterFixedSizeBlob(unsigned byteCount) override;
        virtual unsigned GetVariableSizeBlobCount() const override;
        virtual std::vector<unsigned> const & GetFixedSizeBlobSizes() const override;

    private:

        // The number of variable sized blobs.
        unsigned m_variableSizeBlobCount;

        // Sizes of the fixed-size per document data added by different
        // constituants of the document ingestion. FixedSizeBlobId acts as an
        // index into this array.
        std::vector<unsigned> m_fixedSizeBlobSizes;
    };
}
