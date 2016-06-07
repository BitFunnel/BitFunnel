#pragma once

#include <vector>

#include "BitFunnel/IInterface.h"

namespace BitFunnel
{
    typedef unsigned FixedSizeBlobId;
    typedef unsigned VariableSizeBlobId;

    //*************************************************************************
    //
    // Abstract class or interface for classes that allow the host to register 
    // storage in the DocTable for variable sized or fixed sized fields in the 
    // per document data.
    //
    // Host of the BitFunnel index uses this class to declare a number of
    // blobs of variable and fixed size that they can later fill in with the 
    // per document data during document ingestion. Blobs are referenced
    // using their ids that are returned from the RegisterVariableSizeBlob()
    // and RegisterFixedSizeBlob() functions.
    //
    // This interface defines the schema of the host data. The actual layout of
    // this data is responsibility of the DocTableDescriptor.
    //
    // DESIGN NOTE: Host of the index will create an instance of this class,
    // call its Register* methods multiple times to declare its blobs, then
    // create IIndex. After this it must not call Register* methods again,
    // otherwise the behavior is undefined.
    //
    // DESIGN NOTE: RegisterVariableSizeBlob and RegisterFixedSizeBlob assigns
    // sequential numbers to blob IDs starting from 0.
    //
    // Thread safety: thread safe for multiple readers. Not thread safe for
    // multiple writers.
    //
    //*************************************************************************
    class IDocumentDataSchema : public IInterface
    {
    public:
        // Registers a blob of variable size for host specific per document 
        // data and returns its index.
        virtual VariableSizeBlobId RegisterVariableSizeBlob() = 0;

        // Register a section inside the fixed-size portion of the per document
        // data and returns its id for future use.
        virtual FixedSizeBlobId RegisterFixedSizeBlob(unsigned byteCount) = 0;

        // Returns the number of variable size blobs of per document data defined 
        // in the schema.
        virtual unsigned GetVariableSizeBlobCount() const = 0;

        // Returns the sizes of the fixed sized blobs defined in the schema.
        virtual std::vector<unsigned> const & GetFixedSizeBlobSizes() const = 0;
    };
}