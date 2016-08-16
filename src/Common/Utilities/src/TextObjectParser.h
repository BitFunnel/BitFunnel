#pragma once

#include <istream>
#include <stack>

#include "BitFunnel/IObjectParser.h"
#include "BitFunnel/NonCopyable.h"



namespace BitFunnel
{
    class INode;
    class Term;

    class TextObjectParser : public IObjectParser, NonCopyable
    {
    public:
        typedef int (*TypenameConverter)(const char* name);

        TextObjectParser(std::istream& input,
                         IAllocator& allocator,
                         TypenameConverter typenameConverter);

        IAllocator& GetAllocator() const;

        int ReadTypeTag();

        void OpenObject();
        void OpenObjectField(const char* name);
        void CloseObject();

        void OpenList();
        bool OpenListItem();
        void CloseList();

        void OpenPrimitive(const char* name);
        bool OpenPrimitiveItem();
        void ClosePrimitive();

        bool ParseBool();
        unsigned ParseInt();
        unsigned ParseUInt();
        uint64_t ParseUInt64();
        double ParseDouble();
        char const * ParseStringLiteral();
        void ParseToken(std::string& token);

    private:
        void SkipWhite();
        void Expect(char c);
        void Expect(const char* text);

        IAllocator& m_allocator;
        TypenameConverter m_typenameConverter;

        std::istream& m_input;

        std::stack<unsigned> m_objectFields;
        std::stack<unsigned> m_listItems;
        unsigned m_primitiveItems;
    };
}
