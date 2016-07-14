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

#pragma once

#include <cstring>
#include <ostream>
#include <vector>

#include "Parameter.h"

// C4505 must be enabled for the entire header file and everything that comes
// after because the compiler generates C4505 after parsing all files.
#ifdef _MSC_VER
#pragma warning(disable:4505)
// C4250 is caused by a compiler bug - see http://connect.microsoft.com/VisualStudio/feedback/details/101259/disable-warning-c4250-class1-inherits-class2-member-via-dominance-when-weak-member-is-a-pure-virtual-function
#pragma warning(disable:4250)
#endif // _MSC_VER


namespace CmdLine
{
    template <class T> class IValidator;
    class IConstraint;


    //*************************************************************************
    //
    // Parser
    //
    //*************************************************************************
    class CmdLineParser
    {
    public:
        CmdLineParser(const char* name, const char* description);
        ~CmdLineParser();

        void AddParameter(IRequiredParameter& parameter);
        void AddParameter(IOptionalParameter& parameter);
        void AddConstraint(std::unique_ptr<IConstraint> constraint);

        bool TryParse(std::ostream& error, unsigned argc, char const* const* argv);
        void Usage(std::ostream& out, char const* argv) const;

    private:
        // Attempts to parse an optional parameter starting at argv[currentArg].
        // Returns false if argv[currentArg] matches an optional parameter, but
        // but there are errors parsing the parameter. Otherwise returns true.
        bool ParseOptionalParameter(std::ostream& error, unsigned& currentArg, unsigned argc, char const* const* argv);

        std::string m_name;
        std::string m_description;

        OptionalParameterList m_help;

        std::vector<IRequiredParameter*> m_required;
        std::vector<IOptionalParameter*> m_optional;

        std::vector<IConstraint*> m_constraints;
    };
}
