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

#include <iostream>

#include "CdCommand.h"
#include "Environment.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // Cd - change output directory
    //
    //*************************************************************************
    Cd::Cd(Environment & environment,
           Id id,
           char const * parameters)
        : TaskBase(environment, id, Type::Synchronous)
    {
        m_dir = TaskFactory::GetNextToken(parameters);
    }


    void Cd::Execute()
    {
        GetEnvironment().SetOutputDir(m_dir);
        std::cout
            << "output directory is now \""
            << m_dir
            << "\"." << std::endl;
    }


    ICommand::Documentation Cd::GetDocumentation()
    {
        return Documentation(
            "cd",
            "Change the output directory",
            "cd <path>\n"
            "  Changes the output directory to the specified path.\n"
        );
    }
}
