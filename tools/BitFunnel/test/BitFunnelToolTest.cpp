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

#include <limits>
#include <sstream>
#include <stddef.h>
#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/Data/Sonnets.h"
#include "BitFunnelTool.h"


namespace BitFunnel
{
    // TODO: refactor this copied code into common code.
    TEST(BitFunnelTool, ThreeToolsEndToEndInterpreter)
    {
        //
        // This test is going to run out of a RAM filesystem.
        //
        auto fileSystem = BitFunnel::Factories::CreateRAMFileSystem();
        auto fileManager =
            BitFunnel::Factories::CreateFileManager(
                "config",
                "statistics",
                "index",
                *fileSystem);

        //
        // Initialize RAM filesystem with input files.
        //
        {
            // Open the manifest file.
            auto manifest = fileSystem->OpenForWrite("manifest.txt");

            // Iterate over sequence of Shakespeare sonnet chunk data.
            for (size_t i = 0; i < Sonnets::chunks.size(); ++i)
            {
                // Create chunk file name, and write chunk data.
                std::stringstream name;
                name << "sonnet" << i;
                auto out = fileSystem->OpenForWrite(name.str().c_str());
                out->write(Sonnets::chunks[i].second,
                           static_cast<std::streamsize>(Sonnets::chunks[i].first));

                // Add chunk file to manifest.
                *manifest << name.str() << std::endl;
            }

            auto script = fileSystem->OpenForWrite("testScript");
            *script << "failOnException" << std::endl
                    << "cache chunk sonnet0" << std::endl;
        }

        //
        // Create the BitFunnelTool based on the RAM filesystem.
        //
        BitFunnel::BitFunnelTool tool(*fileSystem);

        //
        // Use the tool to run the statistics builder.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "statistics",
                "manifest.txt",
                "config"
            };

            tool.Main(std::cin,
                      std::cout,
                      static_cast<int>(argv.size()),
                      argv.data());
        }


        //
        // Use the tool to run the TermTable builder.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "termtable",
                "config",
                "0.1",
                "PrivateSharedRank0And3"
            };

            tool.Main(std::cin,
                      std::cout,
                      static_cast<int>(argv.size()),
                      argv.data());
        }


        //
        // Use the tool to run the REPL.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "repl",
                "config",
                // -script and testScript must be on seperate lines because
                // tokens are delimited by whitespace.
                "-script",
                "testScript"
            };

            // Create an input stream with commands to
            // load a chunk, verify a query, and inspect
            // some rows.
            std::stringstream input;
            input
                // This first line is run via -script.
                // << "cache chunk sonnet0" << std::endl
                << "interpreter" << std::endl
                << "verify one blood" << std::endl
                << "show rows blood" << std::endl;

            tool.Main(input,
                      std::cout,
                      static_cast<int>(argv.size()),
                      argv.data());
        }
    }


    TEST(BitFunnelTool, ThreeToolsEndToEnd)
    {
        //
        // This test is going to run out of a RAM filesystem.
        //
        auto fileSystem = BitFunnel::Factories::CreateRAMFileSystem();
        auto fileManager =
            BitFunnel::Factories::CreateFileManager(
                "config",
                "statistics",
                "index",
                *fileSystem);

        //
        // Initialize RAM filesystem with input files.
        //
        {
            // Open the manifest file.
            auto manifest = fileSystem->OpenForWrite("manifest.txt");

            // Iterate over sequence of Shakespeare sonnet chunk data.
            for (size_t i = 0; i < Sonnets::chunks.size(); ++i)
            {
                // Create chunk file name, and write chunk data.
                std::stringstream name;
                name << "sonnet" << i;
                auto out = fileSystem->OpenForWrite(name.str().c_str());
                out->write(Sonnets::chunks[i].second,
                           static_cast<std::streamsize>(Sonnets::chunks[i].first));

                // Add chunk file to manifest.
                *manifest << name.str() << std::endl;
            }

            auto script = fileSystem->OpenForWrite("testScript");
            *script << "failOnException" << std::endl
                    << "cache chunk sonnet0" << std::endl;
        }

        //
        // Create the BitFunnelTool based on the RAM filesystem.
        //
        BitFunnel::BitFunnelTool tool(*fileSystem);

        //
        // Use the tool to run the statistics builder.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "statistics",
                "manifest.txt",
                "config"
            };

            tool.Main(std::cin,
                      std::cout,
                      static_cast<int>(argv.size()),
                      argv.data());
        }


        //
        // Use the tool to run the TermTable builder.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "termtable",
                "config",
                "0.1",
                "PrivateSharedRank0And3"
            };

            tool.Main(std::cin,
                      std::cout,
                      static_cast<int>(argv.size()),
                      argv.data());
        }


        //
        // Use the tool to run the REPL.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "repl",
                "config",
                // -script and testScript must be on seperate lines because
                // tokens are delimited by whitespace.
                "-script",
                "testScript"
            };

            // Create an input stream with commands to
            // load a chunk, verify a query, and inspect
            // some rows.
            std::stringstream input;
            input
                // This first line is run via -script.
                // << "cache chunk sonnet0" << std::endl
                << "verify one blood" << std::endl
                << "show rows blood" << std::endl;

            tool.Main(input,
                      std::cout,
                      static_cast<int>(argv.size()),
                      argv.data());
        }
    }


    // TODO: extract logic into class/function instead of copy-pasting above
    // test.
    TEST(BitFunnelTool, ThreeToolsEndToEndInterpreterMultiShard)
    {
        //
        // This test is going to run out of a RAM filesystem.
        //
        auto fileSystem = BitFunnel::Factories::CreateRAMFileSystem();
        auto fileManager =
            BitFunnel::Factories::CreateFileManager(
                "config",
                "statistics",
                "index",
                *fileSystem);


        //
        // Create ShardDefinition
        //
        {
            // This should cause the test to fail because sharding is
            // incompletely implemented. However, that doesn't seem to happen
            // because it's so incompletely implemented that we don't even read
            // this file. Neither StatisticsBuilder nor TermTableBuilder are
            // shard aware.
            //
            // TODO: have tools actually use ShardDefinition. See #291 for more
            // discussion.

            // According to FileManager, ShardDefinition is in statisticsDirectory.
            auto shardDefinition = fileSystem->OpenForWrite("config/ShardDefinition.csv");
            *shardDefinition << "50";
        }

        //
        // Initialize RAM filesystem with input files.
        //
        {
            // Open the manifest file.
            auto manifest = fileSystem->OpenForWrite("manifest.txt");

            // Iterate over sequence of Shakespeare sonnet chunk data.
            for (size_t i = 0; i < Sonnets::chunks.size(); ++i)
            {
                // Create chunk file name, and write chunk data.
                std::stringstream name;
                name << "sonnet" << i;
                auto out = fileSystem->OpenForWrite(name.str().c_str());
                out->write(Sonnets::chunks[i].second,
                           static_cast<std::streamsize>(Sonnets::chunks[i].first));

                // Add chunk file to manifest.
                *manifest << name.str() << std::endl;
            }

            auto script = fileSystem->OpenForWrite("testScript");
            *script << "failOnException" << std::endl
                    << "cache chunk sonnet0" << std::endl;
        }

        //
        // Create the BitFunnelTool based on the RAM filesystem.
        //
        BitFunnel::BitFunnelTool tool(*fileSystem);

        //
        // Use the tool to run the statistics builder.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "statistics",
                "manifest.txt",
                "config"
            };

            tool.Main(std::cin,
                      std::cout,
                      static_cast<int>(argv.size()),
                      argv.data());
        }


        //
        // Use the tool to run the TermTable builder.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "termtable",
                "config",
                "0.1",
                "PrivateSharedRank0And3"
            };

            tool.Main(std::cin,
                      std::cout,
                      static_cast<int>(argv.size()),
                      argv.data());
        }


        //
        // Use the tool to run the REPL.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "repl",
                "config",
                // -script and testScript must be on seperate lines because
                // tokens are delimited by whitespace.
                "-script",
                "testScript"
            };

            // Create an input stream with commands to
            // load a chunk, verify a query, and inspect
            // some rows.
            std::stringstream input;
            input
                // This first line is run via -script.
                // << "cache chunk sonnet0" << std::endl
                << "interpreter" << std::endl
                << "verify one blood" << std::endl
                << "show rows blood" << std::endl;

            std::stringstream output;
            tool.Main(input,
                      output,
                      static_cast<int>(argv.size()),
                      argv.data());

            // TODO: this is an extremely brittle way to check if we have false
            // positives. This is being done this way because it appears that
            // it's a standard pattern for our commands to entire write to a
            // file or write to a stream. In order to make commands more easily
            // tstable, we should probably return a data structure and have the
            // REPL print out a readable form of the data structure.
            std::string text = output.str();

            // TODO: only print out text on failure.
            std::cout << text;

            size_t false_positive_found = text.find("False positives:");
            if (false_positive_found != std::string::npos)
            {
                FAIL() << "Found false positives.";
            }
        }
    }


    // TODO: extract logic into class/function instead of copy-pasting above
    // test.
    TEST(BitFunnelTool, ThreeToolsEndToEndMultiShard)
    {
        //
        // This test is going to run out of a RAM filesystem.
        //
        auto fileSystem = BitFunnel::Factories::CreateRAMFileSystem();
        auto fileManager =
            BitFunnel::Factories::CreateFileManager(
                "config",
                "statistics",
                "index",
                *fileSystem);


        //
        // Create ShardDefinition
        //
        {
            // This should cause the test to fail because sharding is
            // incompletely implemented. However, that doesn't seem to happen
            // because it's so incompletely implemented that we don't even read
            // this file. Neither StatisticsBuilder nor TermTableBuilder are
            // shard aware.
            //
            // TODO: have tools actually use ShardDefinition. See #291 for more
            // discussion.

            // According to FileManager, ShardDefinition is in statisticsDirectory.
            auto shardDefinition = fileSystem->OpenForWrite("config/ShardDefinition.csv");
            *shardDefinition << "70";
        }

        //
        // Initialize RAM filesystem with input files.
        //
        {
            // Open the manifest file.
            auto manifest = fileSystem->OpenForWrite("manifest.txt");

            // Iterate over sequence of Shakespeare sonnet chunk data.
            for (size_t i = 0; i < Sonnets::chunks.size(); ++i)
            {
                // Create chunk file name, and write chunk data.
                std::stringstream name;
                name << "sonnet" << i;
                auto out = fileSystem->OpenForWrite(name.str().c_str());
                out->write(Sonnets::chunks[i].second,
                           static_cast<std::streamsize>(Sonnets::chunks[i].first));

                // Add chunk file to manifest.
                *manifest << name.str() << std::endl;
            }

            auto script = fileSystem->OpenForWrite("testScript");
            *script << "failOnException" << std::endl
                    << "cache chunk sonnet0" << std::endl;
        }

        //
        // Create the BitFunnelTool based on the RAM filesystem.
        //
        BitFunnel::BitFunnelTool tool(*fileSystem);

        //
        // Use the tool to run the statistics builder.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "statistics",
                "manifest.txt",
                "config"
            };

            tool.Main(std::cin,
                      std::cout,
                      static_cast<int>(argv.size()),
                      argv.data());
        }


        //
        // Use the tool to run the TermTable builder.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "termtable",
                "config",
                "0.1",
                "PrivateSharedRank0And3"
            };

            tool.Main(std::cin,
                      std::cout,
                      static_cast<int>(argv.size()),
                      argv.data());
        }


        //
        // Use the tool to run the REPL.
        //
        {
            std::vector<char const *> argv = {
                "BitFunnel",
                "repl",
                "config",
                // -script and testScript must be on seperate lines because
                // tokens are delimited by whitespace.
                "-script",
                "testScript"
            };

            // Create an input stream with commands to
            // load a chunk, verify a query, and inspect
            // some rows.
            std::stringstream input;
            input
                // This first line is run via -script.
                // << "cache chunk sonnet0" << std::endl
                << "verify one blood" << std::endl
                << "show rows blood" << std::endl;

            std::stringstream output;
            tool.Main(input,
                      output,
                      static_cast<int>(argv.size()),
                      argv.data());

            // TODO: this is an extremely brittle way to check if we have false
            // positives. This is being done this way because it appears that
            // it's a standard pattern for our commands to entire write to a
            // file or write to a stream. In order to make commands more easily
            // tstable, we should probably return a data structure and have the
            // REPL print out a readable form of the data structure.
            std::string text = output.str();

            // TODO: only print out text on failure.
            std::cout << text;

            size_t false_positive_found = text.find("False positives:");
            if (false_positive_found != std::string::npos)
            {
                FAIL() << "Found false positives.";
            }
        }
    }
}
