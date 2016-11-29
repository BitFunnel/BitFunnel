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

#include "BitFunnel/BitFunnelTypes.h"
#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/Exceptions.h"
#include "BitFunnel/IFileManager.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IDocumentFrequencyTable.h"
#include "BitFunnel/Index/IFactSet.h"
#include "BitFunnel/Index/ITermTable.h"
#include "BitFunnel/Index/ITermTableBuilder.h"
#include "BitFunnel/Index/ITermTreatment.h"
#include "BitFunnel/Utilities/Stopwatch.h"
#include "CmdLineParser/CmdLineParser.h"
#include "TermTableBuilderTool.h"


namespace BitFunnel
{
    TermTableBuilderTool::TermTableBuilderTool(IFileSystem& fileSystem)
      : m_fileSystem(fileSystem)
    {
    }


    int TermTableBuilderTool::Main(std::istream& /*input*/,
                               std::ostream& output,
                               int argc,
                               char const *argv[])
    {
        CmdLine::CmdLineParser parser(
            "TermTableBuilderTool",
            "Generate a TermTable from a DocumentFrequencyTable.");

        CmdLine::RequiredParameter<char const *> tempPath(
            "tempPath",
            "Path to a tmp directory. "
            "Something like /tmp/ or c:\\temp\\, depending on platform..");


        parser.AddParameter(tempPath);

        int returnCode = 1;

        if (parser.TryParse(output, argc, argv))
        {
            try
            {
                ShardId shard = 0;
                double density = 0.1;
                double snr = 10.0;
                double adhocFrequency = 0.001;

                BuildTermTable(output,
                               tempPath,
                               shard,
                               density,
                               snr,
                               adhocFrequency);

                returnCode = 0;
            }
            catch (RecoverableError e)
            {
                output << "Error: " << e.what() << std::endl;
            }
            catch (...)
            {
                output << "Unexpected error.";
            }
        }

        return returnCode;
    }


    void TermTableBuilderTool::BuildTermTable(
        std::ostream& output,
        char const * intermediateDirectory,
        ShardId shard,
        double density,
        double snr,
        double adhocFrequency) const
    {
        output << "Loading files for TermTable build." << std::endl;

        auto fileManager = Factories::CreateFileManager(intermediateDirectory,
                                                        intermediateDirectory,
                                                        intermediateDirectory,
                                                        m_fileSystem);

        auto terms(Factories::CreateDocumentFrequencyTable(
            *fileManager->DocFreqTable(shard).OpenForRead()));

        // auto treatment(Factories::CreateTreatmentPrivateSharedRank0And3(
        //     density, snr));

        auto treatment(Factories::CreateTreatmentPrivateSharedRank0(
            density, snr));

        // auto treatment(Factories::CreateTreatmentPrivateSharedRank0ToN(
        //      density, snr));

        auto facts(Factories::CreateFactSet());

        auto termTable(Factories::CreateTermTable());

        output << "Starting TermTable build." << std::endl;

        auto termTableBuilderTool(
            Factories::CreateTermTableBuilder(density,
                                              adhocFrequency,
                                              *treatment,
                                              *terms,
                                              *facts,
                                              *termTable));

        termTableBuilderTool->Print(output);

        output << "Writing TermTable files." << std::endl;

        termTable->Write(*fileManager->TermTable(shard).OpenForWrite());

        output << "Done." << std::endl;
    }
}
