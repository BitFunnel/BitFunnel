#pragma once

#include <string>


namespace BitFunnel
{
    class IAllocator;

    class IObjectParser
    {
    public:
        virtual ~IObjectParser() {};

        virtual IAllocator& GetAllocator() const = 0;

        virtual int ReadTypeTag() = 0;

        // Objects are of the form "name { ... }" where "..." indicates a set
        // of object fields separated by commas. An object field is of the form
        // "fieldname: value" where value is an object, a list, or a
        // primitive.
        virtual void OpenObject() = 0;
        virtual void OpenObjectField(const char* name) = 0;
        virtual void CloseObject() = 0;

        // Lists are of the form "[ ... ]" where "..." indicates a set of
        // objects or a set of primitives separated by commas.
        virtual void OpenList() = 0;
        virtual bool OpenListItem() = 0;
        virtual void CloseList() = 0;

        // Primitives are of the form "name(...)" where "..." indicates a set
        // of parameters separated by commas. Parameters can be numbers or
        // alphanumeric tokens.
        virtual void OpenPrimitive(const char* name) = 0;
        virtual bool OpenPrimitiveItem() = 0;
        virtual void ClosePrimitive() = 0;

        virtual bool ParseBool() = 0;
        virtual unsigned ParseInt() = 0;
        virtual unsigned ParseUInt() = 0;
        virtual uint64_t ParseUInt64() = 0;
        virtual double ParseDouble() = 0;
        virtual char const * ParseStringLiteral() = 0;
        virtual void ParseToken(std::string& token) = 0;
    };
}
