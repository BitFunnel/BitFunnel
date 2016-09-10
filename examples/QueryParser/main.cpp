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

#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Plan/QueryPipeline.h"
#include "BitFunnel/Plan/TermMatchNode.h"
#include "BitFunnel/Utilities/TextObjectFormatter.h"


namespace BitFunnel
{
    void REPL()
    {
        char const * welcome =
            "Welcome to the BitFunnel Query Parser Example.\n"
            "\n"
            "This example is a Read-Eval-Print-Loop (REPL) that reads queries from\n"
            "the console, parses them, and then prints out the resulting tree of\n"
            "TermMatchNodes.\n"
            "\n"
            "Enter a query after the % prompt and press return. To exit the demo\n"
            "just enter a blank line. Here are some query ideas:\n"
            "    Single terms\n"
            "        dog\n"
            "        title:cat\n"
            "    Phrases\n"
            "        \"dogs are your best friend\"\n"
            "        anchors:\"read this awesome page\"\n"
            "    Disjunctions\n"
            "        dogs | cats\n"
            "    Conjunctions\n"
            "        dogs cats\n"
            "        dogs & cats\n"
            "    Negation\n"
            "        -cats\n"
            "    Grouping\n"
            "        dogs (cats | fish)"
            "\n"
            "\n";

        std::cout << welcome;

        QueryPipeline pipeline;

        for (;;)
        {
            try
            {
                std::cout << "% ";
                std::cout.flush();

                std::string line;
                std::getline(std::cin, line);

                if (line.size() == 0)
                {
                    break;
                }

                auto tree = pipeline.ParseQuery(line.c_str());
                if (tree == nullptr)
                {
                    std::cout << "(empty tree)" << std::endl;
                }
                else
                {
                    TextObjectFormatter formatter(std::cout);
                    tree->Format(formatter);
                    std::cout << std::endl << std::endl;
                }
            }
            catch (RecoverableError e)
            {
                std::cout
                    << "Error: "
                    << e.what()
                    << std::endl;
            }
            catch (...)
            {
                std::cout << "Unexpected error. Exiting now." << std::endl;
                throw;
            }
        }

        std::cout << "bye" << std::endl;
    }
}


int main(int /*argc*/, char** /*argv*/)
{
    BitFunnel::REPL();
    return 0;
}
