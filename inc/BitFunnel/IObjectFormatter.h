#pragma once

namespace BitFunnel
{
    class IPersistableObject;

    class IObjectFormatter
    {
    public:
        virtual ~IObjectFormatter() {};

        virtual void OpenObject(const IPersistableObject& object) = 0;
        virtual void OpenObjectField(const char* name) = 0;
        virtual void CloseObject() = 0;

        virtual void NullObject() = 0;

        virtual void OpenList() = 0;
        virtual void OpenListItem() = 0;
        virtual void CloseList() = 0;

        virtual void OpenPrimitive(const char* name) = 0;
        virtual void OpenPrimitiveItem() = 0;
        virtual void ClosePrimitive() = 0;

        virtual void Format(bool value) = 0;
        virtual void Format(int value) = 0;
        virtual void Format(unsigned value) = 0;
        virtual void Format(unsigned long value) = 0;
        virtual void Format(double value) = 0;
        virtual void Format(const char* value) = 0;
        virtual void FormatStringLiteral(const char* value) = 0;
    };
}
