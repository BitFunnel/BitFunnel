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

#include <cstring>
#include <iostream>
#include <memory>

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/Exceptions.h"
#include "BitFunnelTool.h"
#include "REPL.h"
#include "StatisticsBuilder.h"
#include "TermTableBuilderTool.h"


namespace BitFunnel
{
    BitFunnelTool::BitFunnelTool(IFileSystem& fileSystem)
      : m_fileSystem(fileSystem)
    {
    }


    int BitFunnelTool::Main(std::istream& input,
                            std::ostream& output,
                            int argc,
                            char const *argv[])
    {
        int returnCode = 1;

        if (argc < 2)
        {
            Usage(output);
        }
        else if (strcmp(argv[1], "-help") == 0)
        {
            Usage(output);
        }
        else
        {
            auto executable = CreateExecutable(argv[1]);
            if (executable.get() != nullptr)
            {
                std::string name = "BitFunnel ";
                name.append(argv[1]);
                auto args = FilterArgs(argc, argv, name.c_str());
                executable->Main(input,
                                 output,
                                 static_cast<int>(args.size()),
                                 args.data());
                returnCode = 0;
            }
            else
            {
                output
                    << "Unknown command '"
                    << argv[1]
                    << "'. Use 'BitFunnel -help' for more information."
                    << std::endl;
            }
        }

        return returnCode;
    }


    std::unique_ptr<IExecutable>
        BitFunnelTool::CreateExecutable(char const * name) const
    {
        std::unique_ptr<IExecutable> executable;

        if (strcmp(name, "repl") == 0)
        {
            executable.reset(new REPL(m_fileSystem));
        }
        else if (strcmp(name, "statistics") == 0)
        {
            executable.reset(new StatisticsBuilder(m_fileSystem));
        }
        else if (strcmp(name, "termtable") == 0)
        {
            executable.reset(new TermTableBuilderTool(m_fileSystem));
        }

        return executable;
    }


    std::vector<char const *> BitFunnelTool::FilterArgs(
        int argc,
        char const *argv[],
        char const * name) const
    {
        std::vector<char const *> args;
        for (int current = 0; current < argc; ++current)
        {
            if (current == 0)
            {
                args.push_back(name);
            }
            else if (current != 1)
            {
                args.push_back(argv[current]);
            }
        }

        return args;
    }


    void BitFunnelTool::Usage(std::ostream& output)
    {
        output
            << "usage: BitFunnel <command> [<args>]" << std::endl
            << std::endl
            << "The most commonly used commands are" << std::endl
            << "   statistics     Generate corpus statistics used to configure the index." << std::endl
            << "   termtable      Construct a term table based on generated corpus statistics." << std::endl
            << "   repl           Run interative read-eval-print console." << std::endl
            << std::endl
            << "See 'bitfunnel <command> -help' to read about a specific command." << std::endl
            ;
    }
}
