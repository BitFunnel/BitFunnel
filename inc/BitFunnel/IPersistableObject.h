#pragma once

namespace BitFunnel
{
    class IObjectFormatter;

    class IPersistableObject
    {
    public:
        virtual int GetTypeTag() const = 0;
        virtual const char* GetTypeName() const = 0;

        virtual void Format(IObjectFormatter& formatter) const = 0;
    };
}
