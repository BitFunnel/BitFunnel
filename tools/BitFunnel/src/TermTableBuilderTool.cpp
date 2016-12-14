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
#include "BitFunnel/Index/ITermTreatmentFactory.h"
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

        CmdLine::RequiredParameter<char const *> config(
            "config",
            "Path to configuration directory containing files generated "
            "by the 'BitFunnel statistics'.command.");

        CmdLine::RequiredParameter<double> density(
            "density",
            "Target upper bound for bit density.");

        CmdLine::RequiredParameter<char const *> treatment(
            "treatment",
            "Name of the term treatment to use.");


        parser.AddParameter(config);
        parser.AddParameter(density);
        parser.AddParameter(treatment);

        int returnCode = 1;

        if (parser.TryParse(output, argc, argv))
        {
            try
            {
                ShardId shard = 0;
                double snr = 10.0;
                double adhocFrequency = 0.001;

                BuildTermTable(output,
                               config,
                               treatment,
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
                output << "Unexpected error." << std::endl;
            }
        }
        else if (parser.HelpActivated())
        {
            auto treatments = Factories::CreateTreatmentFactory();
            auto names = treatments->GetTreatmentNames();
            auto descriptions = treatments->GetTreatmentDescriptions();

            std::cout << "Available term treatments:" << std::endl;
            for (size_t i = 0; i < names.size(); ++i)
            {
                std::cout << "  " << names[i] << ": " << descriptions[i] << std::endl;
            }
        }

        return returnCode;
    }


    void TermTableBuilderTool::BuildTermTable(
        std::ostream& output,
        char const * configDirectory,
        char const * treatmentName,
        ShardId shard,
        double density,
        double snr,
        double adhocFrequency) const
    {
        output << "Loading files for TermTable build." << std::endl;

        auto fileManager = Factories::CreateFileManager(configDirectory,
                                                        configDirectory,
                                                        configDirectory,
                                                        m_fileSystem);

        auto terms(Factories::CreateDocumentFrequencyTable(
            *fileManager->DocFreqTable(shard).OpenForRead()));

        auto treatments = Factories::CreateTreatmentFactory();
        auto treatment(treatments->CreateTreatment(treatmentName, density, snr));

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
        termTableBuilderTool->Print(*fileManager->TermTableStatistics(shard).OpenForWrite());

        output << "Writing TermTable files." << std::endl;

        termTable->Write(*fileManager->TermTable(shard).OpenForWrite());

        output << "Done." << std::endl;
    }
}
