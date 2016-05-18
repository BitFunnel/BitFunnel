#pragma once

namespace BitFunnel
{
    // IInterface is a base class for all interfaces in BitFunnel.Library.
    // Its sole purpose is to define an empty virtual destructor.
    class IInterface
    {
    public:
        virtual ~IInterface() {}
    };
}
