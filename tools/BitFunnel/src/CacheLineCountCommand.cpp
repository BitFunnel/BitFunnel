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

#include "CacheLineCountCommand.h"
#include "Environment.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // CacheLineCountCommand
    //
    //*************************************************************************
    CacheLineCountCommand::CacheLineCountCommand(Environment & environment,
                                                 Id id,
                                                 char const * /*parameters*/)
        : TaskBase(environment, id, Type::Synchronous)
    {
    }


    void CacheLineCountCommand::Execute()
    {
        auto & env = GetEnvironment();
        env.SetCacheLineCountMode(!env.GetCacheLineCountMode());

        if (env.GetCacheLineCountMode())
        {
            std::cout
                << "Counting cache lines.";
        }
        else
        {
            std::cout
                << "Cache line counting disabled.";
        }
        std::cout
            << std::endl
            << std::endl;
    }


    ICommand::Documentation CacheLineCountCommand::GetDocumentation()
    {
        return Documentation(
            "cachelines",
            "Toggles counting of row cachelines accessed.",
            "cachelines\n"
            "  Toggles counting of row cachelines accessed during query processing."
        );
    }
}
