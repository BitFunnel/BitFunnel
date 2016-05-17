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

#include <algorithm>        // For std::min() or std::max()
#include <sstream>
#include <stdexcept>
#include <stdlib.h>         // For _splitpath_s().
#include <Windows.h>        // For GetModuleFileNameA().

#include "CmdLineParser/CmdLineParser.h"
#include "CmdLineParser/FormattingUtilities.h"


namespace CmdLine
{
    //*************************************************************************
    //
    // CmdLineParser
    //
    //*************************************************************************
    CmdLineParser::CmdLineParser(const char* name, const char* description)
        : m_name(name),
          m_description(description),
          m_help("help", "Display help for this program.")
    {
        AddParameter(m_help);
    }


    CmdLineParser::~CmdLineParser()
    {
        for (unsigned i = 0; i < m_constraints.size(); ++i)
        {
            delete m_constraints[i];
        }
    }


    void CmdLineParser::AddParameter(IRequiredParameter& parameter)
    {
        // Ensure this parameter doesn't conflict with existing parameters.
        for (unsigned i = 0; i < m_required.size(); ++i)
        {
            if (m_required[i]->GetName().compare(parameter.GetName()) == 0)
            {
                throw std::runtime_error("Parameter name already exists.");
            }
        }

        m_required.push_back(&parameter);
    }


    void CmdLineParser::AddParameter(IOptionalParameter& parameter)
    {
        // Ensure this parameter doesn't conflict with existing parameters.
        for (unsigned i = 0; i < m_optional.size(); ++i)
        {
            if (m_optional[i]->GetName().compare(parameter.GetName()) == 0)
            {
                throw std::runtime_error("Parameter name already exists.");
            }
        }

        m_optional.push_back(&parameter);
    }


    void CmdLineParser::AddConstraint(std::auto_ptr<IConstraint> constraint)
    {
        // Release the auto_ptr since its internal pointer will now be stored 
        // in an std::vector<> and released in ArgumentParser::~ArgumentParser().
        m_constraints.push_back(constraint.release());
    }


    // Attempts to parse an optional parameter starting at argv[currentArg].
    // Returns false if argv[currentArg] matches an optional parameter, but
    // but there are errors parsing the parameter. Otherwise returns true.
    bool CmdLineParser::ParseOptionalParameter(std::ostream& error, unsigned& currentArg, 
                                                unsigned argc, char const* const* argv)
    {
        bool success = true;
        for (unsigned i = 0; i < m_optional.size(); ++i)
        {
            if (m_optional[i]->MatchesFlag(argv[currentArg]))
            {
                success = m_optional[i]->TryParse(error, currentArg, argc, argv);
                break;
            }
        }
        return success;
    }


    bool CmdLineParser::TryParse(std::ostream& error, unsigned argc, char const* const* argv)
    {
        bool success = true;
        unsigned currentArg = 1;
        unsigned requiredArg = 0;

        while (success)
        {
            // Parse any optional arguments.
            while (currentArg < argc)
            {
                unsigned oldCurrentArg = currentArg;
                if (argv[currentArg][0] == '-' || argv[currentArg][0] == '/')
                {
                    success = ParseOptionalParameter(error, currentArg, argc, argv);
                    if (!success)
                    {
                        // We encountered an error while attempting to parse an optional parameter.
                        break;
                    }
                    else if (oldCurrentArg == currentArg)
                    {
                        // We didn't encounter any errors, but we didn't find any optional parameters either.
                        // Therefore the current argument must be a required argument.
                        // e.g. parameter was -1 or a the string /foo.
                        break;
                    }
                }
                else
                {
                    break;
                }
            }

            if (!success)
            {
                break;
            }

            if (requiredArg < m_required.size())
            {
                // Drop out of the loop if we've already consumed all the arguments.
                if (currentArg >= argc)
                {
                    // If the user didn't supply the -help flag, report an error.
                    if (!m_help.IsActivated())
                    {
                        error << "Expected ";
                        m_required[requiredArg]->Syntax(error);
                        error << "." << std::endl;
                    }
                    success = false;
                    break;
                }
                else
                {
                    // Attempt to parse the next required argument.
                    if (!m_required[requiredArg]->TryParse(error, currentArg, argc, argv))
                    {
                        success = false;
                        break;
                    }
                    ++requiredArg;
                }
            }
            else
            {
                // Not expecting for any more arguments.
                if (currentArg < argc)
                {
                    error << "Unexpected argument " << argv[currentArg];
                    error << " in position " << currentArg << "." << std::endl;
                    success = false;
                }
                break;
            }
        }

        if (success)
        {
            // Verify that all constraints have been met.
            for (unsigned i = 0; i < m_constraints.size(); ++i)
            {
                success &= m_constraints[i]->IsSatisfied();
                if (!success)
                {
                    error << "Constraint not satisfied: ";
                    m_constraints[i]->Description(error);
                    error << "." << std::endl;
                    break;
                }
            }
        }

        if (m_help.IsActivated())
        {
            Usage(error);

            // Ensure that the presense of help disables all other parameters.
            success = false;
        }
        else if (!success)
        {
            error << "Use -help for more information." << std::endl;
        }

        return success;
    }


    void CmdLineParser::Usage(std::ostream& out) const
    {
        out << m_name << std::endl;

        std::stringstream left;
        std::stringstream right(m_description);

        FormattingUtilities::FormatTextBlock(left, right, out, 0, 80);
        out << std::endl;
        out << std::endl;

        out << "Usage: " << std::endl;

        char path[1000];
        GetModuleFileNameA(NULL, path, sizeof(path));

        char drive[20];
        char dir[1000];
        char name[100];
        char extension[100];
        _splitpath_s(path, drive, dir, name, extension);

        out << name << extension << " ";
        unsigned indent = static_cast<unsigned>(strlen(name)) + static_cast<unsigned>(strlen(extension)) + 1;
        for (unsigned i = 0; i < m_required.size(); ++i)
        {
            if (i > 0)
            {
                FormattingUtilities::Indent(out, indent);
            }
            m_required[i]->Syntax(out);
            out << std::endl;
        }
        for (unsigned i = 0; i < m_optional.size(); ++i)
        {
            if (m_required.size() + i > 0)
            {
                FormattingUtilities::Indent(out, indent);
            }
            m_optional[i]->Syntax(out);
            out << std::endl;
        }
        out << std::endl;

        unsigned leftWidth = 0;
        // Get max over required arguments.
        for (unsigned i = 0; i < m_required.size(); ++i)
        {
            leftWidth = (std::max)(leftWidth, m_required[i]->GetWidth());
        }
        // Then fold in max over optional arguments.
        for (unsigned i = 0; i < m_optional.size(); ++i)
        {
            leftWidth = (std::max)(leftWidth, m_optional[i]->GetWidth());
        }
        leftWidth += 2 + 3 + 1;  // 2 for indentation, 3 for <> or [-], 1 for space after

        unsigned rightWidth = (std::max)(80 - leftWidth, 50u);

        for (unsigned i = 0; i < m_required.size(); ++i)
        {
            m_required[i]->Usage(out, leftWidth, rightWidth);
            out << std::endl;
        }
        for (unsigned i = 0; i < m_optional.size(); ++i)
        {
            m_optional[i]->Usage(out, leftWidth, rightWidth);
            out << std::endl;
        }

        out << std::endl;

        if (m_constraints.size() > 0)
        {
            out << "Constraints" << std::endl;
            for (unsigned i = 0; i < m_constraints.size(); ++i)
            {
                out << "  ";
                m_constraints[i]->Description(out);
                out << "." << std::endl;
            }
        }
    }
}
