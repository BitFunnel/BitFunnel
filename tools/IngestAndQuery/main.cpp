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

#include "CmdLineParser/CmdLineParser.h"
#include "REPL.h"


int main(int argc, char** argv)
{
    CmdLine::CmdLineParser parser(
        "StatisticsBuilder",
        "Ingest documents and compute statistics about them.");

    CmdLine::RequiredParameter<char const *> path(
        "path",
        "Path to a tmp directory. "
        "Something like /tmp/ or c:\\temp\\, depending on platform..");

    // TODO: This parameter should be unsigned, but it doesn't seem to work
    // with CmdLineParser.
    CmdLine::OptionalParameter<int> gramSize(
        "gramsize",
        "Set the maximum ngram size for phrases.",
        1u);

    // TODO: This parameter should be unsigned, but it doesn't seem to work
    // with CmdLineParser.
    CmdLine::OptionalParameter<int> threadCount(
        "threads",
        "Set the thread count for ingestion and query processing.",
        1u);

    parser.AddParameter(path);
    parser.AddParameter(gramSize);
    parser.AddParameter(threadCount);

    int returnCode = 0;

    if (parser.TryParse(std::cout, argc, argv))
    {
        try
        {
            BitFunnel::REPL(path, gramSize, threadCount);
            returnCode = 0;
        }
        catch (...)
        {
            // TODO: Do we really want to catch all exceptions here?
            // Seems we want to at least print out the error message for BitFunnel exceptions.

            std::cout << "Unexpected error.";
            returnCode = 1;
        }
    }
    else
    {
        parser.Usage(std::cout, argv[0]);
        returnCode = 1;
    }

    return returnCode;
}
