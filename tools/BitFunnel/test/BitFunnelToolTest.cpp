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
#include <vector>

#include "gtest/gtest.h"

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/Data/SyntheticChunks.h"
#include "BitFunnelTool.h"


namespace BitFunnel
{
    TEST(BitFunnelTool, ThreeToolsEndToEndSimpleInterpreter)
    {
        //
        // This test is going to run out of a RAM filesystem.
        //
        auto fileSystem = BitFunnel::Factories::CreateRAMFileSystem();
        auto fileManager =
            BitFunnel::Factories::CreateFileManager(
                "config",
                "config",
                "config",
                *fileSystem);

        //
        // Initialize RAM filesystem with input files.
        //
        {
//            auto shardDefinition = Factories::CreateDefaultShardDefinition();
            auto shardDefinition = Factories::CreateShardDefinition();
            shardDefinition->AddShard(32);
            shardDefinition->AddShard(64);

            {
                auto out = fileManager->ShardDefinition().OpenForWrite();
                shardDefinition->Write(*out);
            }

            SyntheticChunks chunks(*shardDefinition, 100, 5);

            // Open the manifest file.
            auto manifest = fileSystem->OpenForWrite("manifest.txt");

            for (size_t i = 0; i < chunks.GetChunkCount(); ++i)
            {
                // Create chunk file name, and write chunk data.
                *manifest << chunks.GetChunkName(i) << std::endl;

                auto out = fileSystem->OpenForWrite(chunks.GetChunkName(i).c_str());
                chunks.WriteChunk(*out, i);
            }

            auto script = fileSystem->OpenForWrite("testScript");
            *script << "failOnException" << std::endl
                    << "cache chunk simpledata0" << std::endl
                    << "cache chunk simpledata1" << std::endl;
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
                //"-script",
                //"testScript"
            };

            // Create an input stream with commands to
            // load a chunk, verify a query, and inspect
            // some rows.
            std::stringstream input;
            input
                // This first line is run via -script.
                // << "cache chunk sonnet0" << std::endl
                << "interpreter" << std::endl
                << "verify one five" << std::endl
                << "show rows five" << std::endl;

            tool.Main(std::cin,
                      std::cout,
                      static_cast<int>(argv.size()),
                      argv.data());
        }
    }
}
