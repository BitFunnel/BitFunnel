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
#include <sstream>

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/Data/Sonnets.h"
#include "BitFunnel/Exceptions.h"
#include "BitFunnelTool.h"
#include "CmdLineParser/CmdLineParser.h"
#include "LoggerInterfaces/Check.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // TheBard is a sample application that configures a BitFunnel index based
    // on a small corpus of 154 Shakespeare sonnets. This standalone example
    // performs an end-to-end configuration, including corpus statistics
    // analysis and TermTable construction.
    //
    // The sonnets are incorporated into the codebase as cstrings, allowing the
    // example to run without touching the filesystem.
    //
    //*************************************************************************
    void Run(bool verbose, bool file)
    {
        //
        // This example uses the RAM filesystem if file is false
        //
        auto fileSystem = file? BitFunnel::Factories::CreateFileSystem()
			: BitFunnel::Factories::CreateRAMFileSystem();
        auto fileManager =
            BitFunnel::Factories::CreateFileManager(
                "config",
                "statistics",
                "index",
                *fileSystem);

        //
        // Initialize RAM/file filesystem with input files.
        //
        {
            std::cout << "Initializing filesystem." << std::endl;

            // Open the manifest file.
            auto manifest = fileSystem->OpenForWrite("manifest.txt");

            // Iterate over sequence of Shakespeare sonnet chunk data.
            for (size_t i = 0; i < Sonnets::chunks.size(); ++i)
            {
                // Create chunk file name, and write chunk data.
                std::stringstream name;
                name << "sonnet" << i;
                auto out = fileSystem->OpenForWrite(name.str().c_str());
                // TODO: consider checking for cast overflow?
                out->write(Sonnets::chunks[i].second,
                           static_cast<std::streamsize>(Sonnets::chunks[i].first));

                // Add chunk file to manifest.
                *manifest << name.str() << std::endl;
            }
        }

        //
        // Create the BitFunnelTool based on the created files.
        //
        BitFunnel::BitFunnelTool tool(*fileSystem);

        //
        // Use the tool to run the statistics builder.
        //
        {
            std::cout << "Gathering corpus statistics." << std::endl;

            std::vector<char const *> argv = {
                "BitFunnel",
                "statistics",
                "manifest.txt",
                "config",
                "-text"
            };

            std::stringstream ignore;
            std::ostream& out = (verbose) ? std::cout : ignore;

            tool.Main(std::cin,
                      out,
                      static_cast<int>(argv.size()),
                      argv.data());
        }


        //
        // Use the tool to run the TermTable builder.
        //
        {
            std::cout << "Building the TermTable." << std::endl;

            std::vector<char const *> argv = {
                "BitFunnel",
                "termtable",
                "config",
                "0.1",
                "PrivateSharedRank0ToN"
            };

            std::stringstream ignore;
            std::ostream& out = (verbose) ? std::cout : ignore;

            auto result = tool.Main(std::cin,
                                    out,
                                    static_cast<int>(argv.size()),
                                    argv.data());

            CHECK_EQ(result, 0) << "TermTableBuilder failed.";

            std::cout
                << "Index is now configured."
                << std::endl
                << std::endl;
        }


        //
        // Use the tool to run the REPL.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "repl",
                "config"
            };

            tool.Main(std::cin,
                      std::cout,
                      static_cast<int>(argv.size()),
                      argv.data());
        }
    }
}


int main(int argc, const char *const *argv)
{
    CmdLine::CmdLineParser parser(
        "TheBard",
        "A small end-to-end index configuration and ingestion example "
        "based on 154 Shakespeare sonnets.");

    CmdLine::OptionalParameterList verbose(
        "verbose",
        "Print information gathered during statistics and "
        "termtable stages.");

	CmdLine::OptionalParameterList file(
		"file",
		"Produce sonnet configuration as real files (vs. memory-only operation).");

	// TODO: This parameter should be unsigned, but it doesn't seem to work
    // with CmdLineParser.
    CmdLine::OptionalParameter<int> gramSize(
        "gramsize",
        "Set the maximum n-gram size for phrases.",
        1u);

    parser.AddParameter(verbose);
	parser.AddParameter(file);
    parser.AddParameter(gramSize);

    int returnCode = 1;

    if (parser.TryParse(std::cout, argc, argv))
    {
        try
        {
            BitFunnel::Run(verbose.IsActivated(), file.IsActivated());
        }
        catch (BitFunnel::RecoverableError e)
        {
            std::cout << "Error: " << e.what() << std::endl;
        }
        catch (...)
        {
            // TODO: Do we really want to catch all exceptions here?
            // Seems we want to at least print out the error message for BitFunnel exceptions.

            std::cout << "Unexpected error." << std::endl;
        }
    }

    return returnCode;
}
