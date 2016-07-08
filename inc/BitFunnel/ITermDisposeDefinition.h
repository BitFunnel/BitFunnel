#pragma once

#include <ostream>

#include "BitFunnel/BitFunnelTypes.h"   // IdfX10, Tier are parameters.


namespace BitFunnel
{
    //*************************************************************************
    //
    // ITermDisposeDefinition is an abstract base class or interface for classes
    // checking whether a term should be discarded or not. 
    //
    //*************************************************************************
    class ITermDisposeDefinition
    {
    public:
        virtual ~ITermDisposeDefinition() {};

        virtual bool IsDispose(unsigned gramSize, IdfX10 idfSum, Tier tierHint) const = 0;

        virtual void Write(std::ostream& output) const = 0;
    };
}
