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
        // Not thread safe - the caller must maintain thread safety.
        virtual void Recycle() = 0;
    };
}
