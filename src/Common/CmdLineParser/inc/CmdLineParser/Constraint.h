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

#include <memory>           // For std::unique_ptr
#include <ostream>

#include "BitFunnel/NonCopyable.h"    // CoexistenceConstraint inherits from noncopyable.


namespace CmdLine
{
    class IOptionalParameter;

    //*************************************************************************
    //
    // IConstraint
    //
    // Abstract bases class for describing a constraint relating to the existence
    // of parameters on the command line (e.g. -one and -two are mutually exclusive).
    //
    //*************************************************************************
    class IConstraint
    {
    public:
        virtual ~IConstraint() {}
        virtual bool IsSatisfied() const = 0;
        virtual void Description(std::ostream& out) const = 0;
    };

    //*************************************************************************
    //
    // CoexistenceConstraint
    //
    // Constraint relating to the coexistence of two parameters (e.g. mutally
    // exclusive, mutually required, one depends upon the other, etc.).
    //
    //*************************************************************************
    class CoexistenceConstraint : public IConstraint, public BitFunnel::NonCopyable
    {
    public:
        enum CoexistenceTypeEnum {MutuallyExclusive, MutuallyRequired, Implies};

        CoexistenceConstraint(CoexistenceTypeEnum t, const IOptionalParameter& a, const IOptionalParameter& b);
        bool IsSatisfied() const;
        void Description(std::ostream& out) const;

    private:
        const IOptionalParameter& m_parameter1;
        const IOptionalParameter& m_parameter2;
        CoexistenceTypeEnum m_type;
    };


    std::unique_ptr<IConstraint> MutuallyExclusive(const IOptionalParameter& a, const IOptionalParameter& b);
    std::unique_ptr<IConstraint> MutuallyRequired(const IOptionalParameter& a, const IOptionalParameter& b);
}
