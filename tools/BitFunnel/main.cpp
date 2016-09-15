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

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Configuration/IFileSystem.h"
//#include "CmdLineParser/CmdLineParser.h"
#include "REPL.h"


void Usage()
{
    std::cout
        << "usage: BitFunnel <command> [<args>]" << std::endl
        << std::endl
        << "The most commonly used commands are" << std::endl
        << "   statistics     Generate corpus statistics used to configure the index." << std::endl
        << "   termtable      Construct a term table based on generated corpus statistics." << std::endl
        << "   repl           Run interative read-eval-print console." << std::endl
        << std::endl
        << "'bitfunnel help' lists available subcommands. See 'bitfunnel help <command> to read" << std::endl
        << "about a specific command." << std::endl
        ;
}


int main(int argc, char** argv)
{
    //CmdLine::CmdLineParser parser(
    //    "BitFunnel",
    //    "Tools for configuring and running BitFunnel.");

    //CmdLine::RequiredParameter<char const *> command(
    //    "command",
    //    "Command to run: "
    //    "statistics termtable repl");


    //parser.AddParameter(command);

    int returnCode = 1;

    //if (parser.TryParse(std::cout, argc, argv))
    if (argc < 2)
    {
        Usage();
    }
    else
    {
        auto fileSystem = BitFunnel::Factories::CreateFileSystem();
        if (strcmp("repl", argv[1]))
        {
            BitFunnel::REPL repl(*fileSystem);
            repl.Main(argc, argv);
            returnCode = 0;
        }
        else
        {
            Usage();
        }
    }

    return returnCode;
}
