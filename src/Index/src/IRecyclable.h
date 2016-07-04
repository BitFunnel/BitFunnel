#pragma once

#include "BitFunnel/IInterface.h"

namespace BitFunnel
{
    //*************************************************************************
    //
    // Abstract class or interface for classes that rely on offline garbage 
    // collection to release their resources after all consumers of the 
    // resource have been drained. Implementors will need to provide a way
    // to indicate if this object can be recycled, as well as the function
    // to call to recycle.
    //
    //*************************************************************************
    class IRecyclable : public IInterface
    {
    public:
        // Tries to recycle a resource. Returns true if this resource was 
        // successfully recycled. Returns false if the resource could not be
        // recycled now and the caller may retry later. A resource may not be
        // recycled if there are possible consumers of it which have not yet
        // exited. Once the methods returns true, it may not be called again.
        // Not thread safe - the caller must maintain thread safety.
        virtual bool TryRecycle() = 0;
    };
}