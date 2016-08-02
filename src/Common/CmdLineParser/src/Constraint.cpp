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


#include <stdexcept>
#include <string>

#include "CmdLineParser/Constraint.h"
#include "CmdLineParser/Parameter.h"


namespace CmdLine
{
    //*************************************************************************
    //
    // CoexistenceConstraint
    //
    //*************************************************************************
    CoexistenceConstraint::CoexistenceConstraint(CoexistenceTypeEnum t,
                                                 const IOptionalParameter& a,
                                                 const IOptionalParameter& b)
        : m_parameter1(a),
          m_parameter2(b),
          m_type(t)
    {
        if (m_type != MutuallyExclusive &&
            m_type != MutuallyRequired &&
            m_type != Implies)
        {
            throw std::runtime_error("Bad Coexistence Type.");
        }
    }


    bool CoexistenceConstraint::IsSatisfied() const
    {
        bool success = false;

        switch (m_type)
        {
        case MutuallyExclusive:
            // True if one or both have not been activated.
            success = !(m_parameter1.IsActivated() & m_parameter2.IsActivated());
            break;
        case MutuallyRequired:
            // True neither or both have been activated.
            success = !(m_parameter1.IsActivated() ^ m_parameter2.IsActivated());
            break;
        case Implies:
            // First parameter requires that second be activated.
            success = (!m_parameter1.IsActivated() || m_parameter2.IsActivated());
            break;
        }

        return success;
    }


    void CoexistenceConstraint::Description(std::ostream& out) const
    {
        switch (m_type)
        {
        case MutuallyExclusive:
            out << "Optional parameter -" << m_parameter1.GetName() << " is incompatible with -" << m_parameter2.GetName();
            break;
        case MutuallyRequired:
            out << "Optional parameters -" << m_parameter1.GetName() << " and -" << m_parameter2.GetName() << " must be used together.";
            break;
        case Implies:
            out << "Optional parameter -" << m_parameter1.GetName() << " requires -" << m_parameter2.GetName();
            break;
        }
    }


#define DEFINE_COEXISTENCE_CONSTRAINT(TYPE)\
    std::unique_ptr<IConstraint> TYPE(const IOptionalParameter& a, const IOptionalParameter& b)\
    {\
        return std::unique_ptr<IConstraint>(new CoexistenceConstraint(CoexistenceConstraint::TYPE, a, b));\
    }

    DEFINE_COEXISTENCE_CONSTRAINT(MutuallyExclusive)
    DEFINE_COEXISTENCE_CONSTRAINT(MutuallyRequired)

#undef DEFINE_COEXISTENCE_CONSTRAINT
}
