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

#include <cstddef>      // size_t parameter.

#include "ITask.h"      // Base class.


namespace BitFunnel
{
    class Environment;

    class ICommand : public ITask
    {
    public:
        typedef size_t Id;

        enum Type {
            Synchronous,
            Asynchronous,
            Exit
        };


        virtual Type GetType() const = 0;
        virtual Id GetId() const = 0;
        virtual Environment & GetEnvironment() const = 0;


        class Documentation
        {
        public:
            Documentation(char const * name,
                          char const * parameters,
                          char const * description)
                : m_name(name),
                m_parameters(parameters),
                m_description(description)
            {
            }

            char const * GetName() const
            {
                return m_name;
            }

            char const * GetParameters() const
            {
                return m_parameters;
            }

            char const * GetDescription() const
            {
                return m_description;
            }

        private:
            char const * m_name;
            char const * m_parameters;
            char const * m_description;
        };
    };
}
