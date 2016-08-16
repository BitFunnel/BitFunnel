#include <iomanip>

#include "BitFunnel/IPersistableObject.h"
#include "BitFunnel/Term.h"
#include "TextObjectFormatter.h"

namespace BitFunnel
{
    namespace Factories
    {
        IObjectFormatter* CreateObjectFormatter(std::ostream& output)
        {
            return new TextObjectFormatter(output);
        }
    }

    TextObjectFormatter::TextObjectFormatter(std::ostream& output)
        : m_output(output),
          m_indentation(0)
    {
    }


    TextObjectFormatter::TextObjectFormatter(std::ostream& output, unsigned indentation)
        : m_output(output),
          m_indentation(indentation)
    {
    }


    void TextObjectFormatter::OpenObject(const IPersistableObject& object)
    {
        m_output << object.GetTypeName() << " {" << std::endl;
        ++m_indentation;
        m_objectFields.push(0);
    }


    void TextObjectFormatter::OpenObjectField(const char* name)
    {
        if (m_objectFields.top() > 0)
        {
            m_output << "," << std::endl;
        }
        m_objectFields.top()++;
        Indent();
        m_output << name << ": ";
    }


    void TextObjectFormatter::CloseObject()
    {
        --m_indentation;
        m_objectFields.pop();
        m_output << std::endl;
        Indent();
        m_output << "}";
    }


    void TextObjectFormatter::NullObject()
    {
        m_output << "";
    }


    void TextObjectFormatter::OpenList()
    {
        m_output << "[";
        ++m_indentation;
        m_listItems.push(0);
    }


    void TextObjectFormatter::OpenListItem()
    {
        if (m_listItems.top() > 0)
        {
            m_output << ",";
        }
        m_output << std::endl;
        m_listItems.top()++;
        Indent();
    }


    void TextObjectFormatter::CloseList()
    {
        m_output << std::endl;
        --m_indentation;
        m_listItems.pop();
        Indent();
        m_output << "]";
    }


    void TextObjectFormatter::OpenPrimitive(const char* name)
    {
        m_output << name << "(";
        m_primitiveItems = 0;
    }


    void TextObjectFormatter::OpenPrimitiveItem()
    {
        if (m_primitiveItems > 0)
        {
            m_output << ", ";
        }
        m_primitiveItems++;
    }


    void TextObjectFormatter::ClosePrimitive()
    {
        m_output << ")";
    }


    // TODO: Consider using template-specialized helper function.
    void TextObjectFormatter::Format(bool value)
    {
        if (value)
        {
            m_output << "true";
        }
        else
        {
            m_output << "false";
        }
    }


    void TextObjectFormatter::Format(int value)
    {
        m_output << value;
    }


    void TextObjectFormatter::Format(unsigned value)
    {
        m_output << value;
    }


    void TextObjectFormatter::Format(unsigned long value)
    {
        m_output << value;
    }


    void TextObjectFormatter::Format(double value)
    {
        m_output << value;
    }


    void TextObjectFormatter::Format(const char* value)
    {
        m_output << value;
    }


    void TextObjectFormatter::FormatStringLiteral(const char* value)
    {
        m_output << '"';

        for (const char* current = value; *current != '\0'; ++current)
        {
            if (*current == '\\' || *current == '"')
            {
                m_output << '\\';
            }
            m_output << *current;
        }

        m_output << '"';
    }


    void TextObjectFormatter::Indent()
    {
        m_output << std::setfill(' ') << std::setw(m_indentation * 2) << "";
    }
}
