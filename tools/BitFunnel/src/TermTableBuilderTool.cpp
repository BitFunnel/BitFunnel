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
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IDocumentFrequencyTable.h"
#include "BitFunnel/Index/ITermTable.h"
#include "BitFunnel/Index/ITermTableBuilder.h"
#include "BitFunnel/Index/ITermTreatmentFactory.h"
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

        CmdLine::OptionalParameter<double> snr(
            "snr",
            "Set signal-to-noise ratio.",
            10.0,
            CmdLine::GreaterThan(0.0));


        parser.AddParameter(config);
        parser.AddParameter(density);
        parser.AddParameter(treatment);
        parser.AddParameter(snr);

        int returnCode = 1;

        if (parser.TryParse(output, argc, argv))
        {
            auto fileManager = Factories::CreateFileManager(config,
                                                            config,
                                                            config,
                                                            m_fileSystem);


            // Pull out ShardDefinition to get number of shards. Equivalent code
            // also exists in SimpleIndex, but we don't want to use that
            // directly since we don't want to stand up an Index/Ingestor to
            // build the TermTable.
            ShardId shardCount = 0;
            {
                auto input = fileManager->ShardDefinition().OpenForRead();
                auto shardDefinition = Factories::CreateShardDefinition(*input);
                shardCount = shardDefinition->GetShardCount();
            }

            try
            {
                double adhocFrequency = density;

                // Check if treatmentName starts with Classic. This is a bit of
                // a hack and we should probably just take adhocFrequency
                // directly.
                std::string treatmentName(treatment);
                std::string classic("Classic");
                if (treatmentName.compare(0, classic.length(), classic) == 0)
                {
                    adhocFrequency = 1.0;
                }

                std::string optimal("Optimal");
                if (treatmentName.compare(0, optimal.length(), optimal) == 0)
                {
                    adhocFrequency = 0.01;
                }


                for (ShardId shard = 0; shard < shardCount; ++shard)
                {

                    BuildTermTable(output,
                                   *fileManager,
                                   treatment,
                                   shard,
                                   density,
                                   snr,
                                   adhocFrequency);
                }

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
        IFileManager& fileManager,
        char const * treatmentName,
        ShardId shard,
        double density,
        double snr,
        double adhocFrequency) const
    {
        output << "Loading files for TermTable build: "
               << shard << std::endl;

        auto terms(Factories::CreateDocumentFrequencyTable(
            *fileManager.DocFreqTable(shard).OpenForRead()));

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
        termTableBuilderTool->Print(*fileManager.TermTableStatistics(shard).OpenForWrite());

        output << "Writing TermTable files." << std::endl;

        termTable->Write(*fileManager.TermTable(shard).OpenForWrite());

        output << "Done." << std::endl;
    }
}
