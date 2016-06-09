#pragma once

#include <iosfwd>

#include "BitFunnel/DocumentHandle.h"
#include "BitFunnel/IInterface.h"


namespace BitFunnel
{
    class IIngestionIndex;

    class IDocument : public IInterface
    {
    public:
        // Requests that the IIndex allocates a DocumentHandle for the 
        // document. Throws if out of space.
        //
        // TODO: other naming ideas - Route, RouteMe, Allocate, Forward, 
        // PrepareToIngest, OpenForIngestion?
        virtual DocumentHandle Place(IIngestionIndex& index) const = 0;

        // Ingests the contents of this document into the index.
        // DocumentHandle is valid only for the entire duration of the Ingest()
        // method.
        virtual void Ingest(DocumentHandle handle) const = 0;

        // Writes the contents of the IDocument to a stream.
        // TODO: use IInputStream when integrated with the reset of 
        // BitFunnel.Library
        virtual void Write(std::ostream& output) const = 0;
    };
}
