#include <cstring>
#include <iomanip>

#include "BitFunnel/Allocators/IAllocator.h"
#include "LoggerInterfaces/Logging.h"
#include "BitFunnel/Term.h"
#include "TextObjectParser.h"

namespace BitFunnel
{
    TextObjectParser::TextObjectParser(std::istream& input,
                                       IAllocator& allocator,
                                       TypenameConverter typenameConverter)
        : m_allocator(allocator),
          m_typenameConverter(typenameConverter),
          m_input(input)
    {
    }


    IAllocator& TextObjectParser::GetAllocator() const
    {
        return m_allocator;
    }


    int TextObjectParser::ReadTypeTag()
    {
        std::string token;
        ParseToken(token);

        return m_typenameConverter(token.c_str());
    }


    void TextObjectParser::OpenObject()
    {
        Expect('{');
        m_objectFields.push(0);
    }


    void TextObjectParser::OpenObjectField(const char* name)
    {
        if (m_objectFields.top() > 0)
        {
            Expect(',');
        }
        m_objectFields.top()++;

        std::string token;
        ParseToken(token);

        // TODO: what does compare do?
        LogAssertB(token.compare(name) == 0, "");

        Expect(':');
    }


    void TextObjectParser::CloseObject()
    {
        m_objectFields.pop();
        Expect('}');
    }


    void TextObjectParser::OpenList()
    {
        Expect('[');
        m_listItems.push(0);
    }


    bool TextObjectParser::OpenListItem()
    {
        bool moreItems = false;

        SkipWhite();
        if (m_input.peek() != ']')
        {
            moreItems = true;
            if (m_listItems.top() > 0)
            {
                Expect(',');
            }
            m_listItems.top()++;
        }

        return moreItems;
    }


    void TextObjectParser::CloseList()
    {
        m_listItems.pop();
        Expect(']');
    }


    void TextObjectParser::OpenPrimitive(const char* name)
    {
        Expect(name);
        Expect('(');
        m_primitiveItems = 0;
    }


    bool TextObjectParser::OpenPrimitiveItem()
    {
        bool moreItems = false;

        SkipWhite();
        if (m_input.peek() != ')')
        {
            moreItems = true;
            if (m_primitiveItems > 0)
            {
                Expect(',');
            }
            m_primitiveItems++;
        }

        return moreItems;
    }


    void TextObjectParser::ClosePrimitive()
    {
        Expect(')');
    }


    // TODO: Consider using template-specialized helper function.
    bool TextObjectParser::ParseBool()
    {
        std::string token;
        ParseToken(token);

        bool result = false;

        if (token.compare("true") == 0)
        {
            result = true;
        }
        else if (token.compare("false") != 0)
        {
            // TODO: what does compare do?
            LogAbortB("");
        }

        return result;
    }


    unsigned TextObjectParser::ParseInt()
    {
        int value = 0;
        m_input >> value;
        LogAssertB(!m_input.fail(), "Unexpected input for ParseInt");
        return value;
    }


    unsigned TextObjectParser::ParseUInt()
    {
        unsigned value = 0;
        m_input >> value;
        LogAssertB(!m_input.fail(), "Unexpected input for ParseUInt");
        return value;
    }


    uint64_t TextObjectParser::ParseUInt64()
    {
        uint64_t value = 0;
        m_input >> value;
        LogAssertB(!m_input.fail(), "Unexpected input for ParseUint64");
        return value;
    }


    double TextObjectParser::ParseDouble()
    {
        double value = 0;
        m_input >> value;
        LogAssertB(!m_input.fail(), "Unexpected input for ParseDouble");
        return value;
    }


    char const * TextObjectParser::ParseStringLiteral()
    {
        SkipWhite();
        Expect('"');

        std::string resultString;
        char currentChar = static_cast<char>(m_input.get());
        while (currentChar != '"')
        {
            if (currentChar == '\\')
            {
                currentChar = static_cast<char>(m_input.get());
            }
            resultString.push_back(currentChar);
            currentChar = static_cast<char>(m_input.get());
        }

        size_t bufferSize = resultString.size() + 1;
        char* result = static_cast<char*>(m_allocator.Allocate(sizeof(char) * bufferSize));
        strcpy(result, resultString.c_str());
        result[bufferSize - 1] = '\0';

        return result;
    }


    void TextObjectParser::ParseToken(std::string& token)
    {
        SkipWhite();
        while (isalnum(m_input.peek()))
        {
            token.push_back(static_cast<char>(m_input.get()));
        }
    }


    void TextObjectParser::SkipWhite()
    {
        while (isspace(m_input.peek()))
        {
            m_input.get();
        }
    }


    void TextObjectParser::Expect(char c)
    {
        SkipWhite();
        LogAssertB(m_input.peek() == c, "Unexpected character");
        m_input.get();
    }


    void TextObjectParser::Expect(const char* text)
    {
        SkipWhite();
        while (*text != '\0' && *text == m_input.peek())
        {
            text++;
            m_input.get();
        }
        LogAssertB(*text == '\0', "Missing null byte");
    }
}
