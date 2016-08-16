#pragma once

#include <ostream>
#include <stack>

#include "BitFunnel/NonCopyable.h"
#include "BitFunnel/IObjectFormatter.h"


namespace BitFunnel
{
    class Term;


    class TextObjectFormatter : public IObjectFormatter, NonCopyable
    {
    public:
        TextObjectFormatter(std::ostream& output);
        TextObjectFormatter(std::ostream& output, unsigned indentation);

        void OpenObject(const IPersistableObject& object);
        void OpenObjectField(char const * name);
        void CloseObject();

        void NullObject();

        void OpenList();
        void OpenListItem();
        void CloseList();

        void OpenPrimitive(char const * name);
        void OpenPrimitiveItem();
        void ClosePrimitive();

        void Format(bool value);
        void Format(int value);
        void Format(unsigned value);
        void Format(unsigned long value);
        void Format(double value);
        void Format(char const * value);
        void FormatStringLiteral(char const * value);

    private:
        void Indent();

        std::ostream& m_output;
        unsigned m_indentation;

        std::stack<unsigned> m_objectFields;
        std::stack<unsigned> m_listItems;
        unsigned m_primitiveItems;
    };
}
