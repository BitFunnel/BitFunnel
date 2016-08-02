// The MIT License (MIT)

// Copyright (c) 2016, Microsoft

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <algorithm>    // for std::min() or std::max()
#include <sstream>
#include <string.h>     // for strcmp, NULL
#include <stdexcept>    // for runtime_error
#include <string>       // for operator<<, string
#include <vector>       // for vector

#include "CmdLineParser/FormattingUtilities.h"
#include "CmdLineParser/Parameter.h"


namespace CmdLine
{
    //*************************************************************************
    //
    // ParameterBase
    //
    //*************************************************************************
    ParameterBase::ParameterBase(const char* name, const char* description)
        : m_isActivated(false),
          m_name(name),
          m_description(description)

    {
        if (name == NULL)
        {
            throw std::runtime_error("Name cannot be NULL.");
        }
        if (description == NULL)
        {
            throw std::runtime_error("Description cannot be NULL.");
        }
        if (name[0] == '-')
        {
            throw std::runtime_error("The name cannot start with -.");
        }
        if (name[0] == '/')
        {
            throw std::runtime_error("The name cannot start with /.");
        }
    }


    const std::string& ParameterBase::GetName() const
    {
        return m_name;
    }


    unsigned ParameterBase::GetWidth() const
    {
        // TODO: check cast to unsigned.
        return static_cast<unsigned>(m_name.size());
    }


    bool ParameterBase::IsActivated() const
    {
        return m_isActivated;
    }


    void ParameterBase::Usage(std::ostream& out, unsigned leftMargin, unsigned targetWidth) const
    {
        Syntax(out);
        out << std::endl;

        std::stringstream left;
        std::stringstream right;
        Description(right);

        FormattingUtilities::FormatTextBlock(left, right, out, 4, targetWidth - leftMargin);
        out << std::endl;
    }


    void ParameterBase::Description(std::ostream& out) const
    {
        out << m_description;
    }


    template <>
    const char* ParameterBase::GetTypeName<int>() const
    {
        return "integer";
    }


    template<>
    const char* ParameterBase::GetTypeName<double>() const
    {
        return "floating point";
    }


    template<>
    const char* ParameterBase::GetTypeName<bool>() const
    {
        return "boolean";
    }


    template<>
    const char* ParameterBase::GetTypeName<const char*>() const
    {
        return "string";
    }


    bool ParameterBase::TryParse(const char* input, int& value)
    {
        bool success = true;

        // Check to make sure the input actually represents an integer.
        // This check is necessary because the >> operator will accept
        // floating point numbers like 2.1.
        const char* current = input;
        if (*current == '-')
        {
            current++;
        }
        else if (*current == '+')
        {
            current++;
        }
        while (*current != 0)
        {
            if (*current > '9' || *current < '0')
            {
                success = false;
                break;
            }
            current++;
        }

        if (success)
        {
            std::stringstream ss(input);
            ss >> value;
            success = !ss.fail();
        }

        return success;
    }


    bool ParameterBase::TryParse(const char* input, double& value)
    {
        std::stringstream ss(input);
        ss >> value;

        return !ss.fail();
    }


    bool ParameterBase::TryParse(const char* input, const char*& value)
    {
        value = input;
        return true;
    }


    bool ParameterBase::TryParse(const char* input, bool& value)
    {
        bool success = false;

        if (strcmp(input, "true") == 0)
        {
            value = true;
            success = true;
        }
        else if (strcmp(input, "false") == 0)
        {
            value = false;
            success = true;
        }

        return success;
    }


    void ParameterBase::Format(std::ostream& out, int value)
    {
        out << value;
    }


    void ParameterBase::Format(std::ostream& out, double value)
    {
        out << value;
    }


    void ParameterBase::Format(std::ostream& out, bool value)
    {
        if (value)
        {
            out << "true";
        }
        else
        {
            out << "false";
        }
    }


    void ParameterBase::Format(std::ostream& out, const char* value)
    {
        if (value != NULL)
        {
            out << '"' << value << '"';
        }
        else
        {
            out << "NULL";
        }
    }


    //*************************************************************************
    //
    // OptionalParameterList
    //
    //*************************************************************************
    OptionalParameterList::OptionalParameterList(const char* name,
                                                 const char* description)
        : OptionalParameter<bool>(name, description, false)
    {
    }


    void OptionalParameterList::AddParameter(IRequiredParameter& parameter)
    {
        // Ensure this parameter doesn't conflict with existing parameters.
        for (unsigned i = 0; i < m_parameters.size(); ++i)
        {
            if (m_parameters[i]->GetName().compare(parameter.GetName()) == 0)
            {
                throw std::runtime_error("Parameter name already exists.");
            }
        }

        m_parameters.push_back(&parameter);
    }


    unsigned OptionalParameterList::GetWidth() const
    {
        unsigned width = 0;
        for (unsigned i = 0 ; i < m_parameters.size(); ++i)
        {
            width = (std::max)(width, m_parameters[i]->GetWidth());
        }
        return width;
    }


    bool OptionalParameterList::TryParse(std::ostream& error, unsigned& currentArg, unsigned argc, char const* const* argv)
    {
        bool success = true;

        if (m_isActivated)
        {
            success = false;
            error << "Found second instance of optional argument ";
            Syntax(error);
            error << "." << std::endl;
        }
        else
        {
            ++currentArg;

            for (unsigned i = 0; i < m_parameters.size(); ++i)
            {
                if (!m_parameters[i]->TryParse(error, currentArg, argc, argv))
                {
                    success = false;
                    break;
                }
            }
            }

        if (success)
        {
            SetValue(true);
            m_isActivated = true;
        }

        return success;
    }


    void OptionalParameterList::Syntax(std::ostream& out) const
    {
        out << "[-" << GetName();
        for (unsigned i = 0; i < m_parameters.size(); ++i)
        {
            out << ' ';
            m_parameters[i]->Syntax(out);
        }
        out << "]";
    }


    void OptionalParameterList::Usage(std::ostream& out, unsigned leftWidth, unsigned rightWidth) const
    {
        if (m_parameters.size() == 0)
        {
            Syntax(out);
            out << std::endl;

            std::stringstream left;

            std::stringstream right;
            Description(right);
            FormattingUtilities::FormatTextBlock(left, right, out, 4, 80 - 4);

            out << std::endl << std::endl;
        }
        else
        {
            Syntax(out);
            out << std::endl;

            std::stringstream left;
            std::stringstream right;
            Description(right);
            FormattingUtilities::FormatTextBlock(left, right, out, 4, 80 - 4);

            out << std::endl << std::endl;

            for (unsigned i = 0; i < m_parameters.size(); ++i)
            {
                std::stringstream left2;
                left2 << "  <" << m_parameters[i]->GetName();
                left2 << ">";

                std::stringstream right2;
                m_parameters[i]->Description(right2);

                FormattingUtilities::FormatTextBlock(left2, right2, out, leftWidth, rightWidth);

                out << std::endl;
                out << std::endl;
            }
        }
    }
}
