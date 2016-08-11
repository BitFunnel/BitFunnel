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
#include "BitFunnel/IFileManager.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IDocumentFrequencyTable.h"
#include "BitFunnel/ITermTable2.h"
#include "BitFunnel/Index/ITermTableBuilder.h"
#include "BitFunnel/ITermTreatment.h"
#include "CmdLineParser/CmdLineParser.h"


namespace BitFunnel
{
    void BuildTermTable(char const * intermediateDirectory,
                        ShardId shard,
                        double density,
                        double snr,
                        double adhocFrequency)
    {
        auto fileManager = Factories::CreateFileManager(intermediateDirectory,
                                                        intermediateDirectory,
                                                        intermediateDirectory);

        auto terms(Factories::CreateDocumentFrequencyTable(*fileManager->DocFreqTable(shard).OpenForRead()));

        auto treatment(Factories::CreateTreatmentPrivateShardRank0And3(density, snr));

        auto termTable(Factories::CreateTermTable());

        auto termTableBuilder(Factories::CreateTermTableBuilder(density,
                                                                adhocFrequency,
                                                                *treatment,
                                                                *terms,
                                                                *termTable));

        termTableBuilder->Print(std::cout);

        termTable->Write(*fileManager->TermTable(shard).OpenForWrite());
    }
}

int main(int argc, char** argv)
{
    CmdLine::CmdLineParser parser(
        "TermTableBuilder",
        "Generate a TermTable from a DocumentFrequencyTable.");

    CmdLine::RequiredParameter<char const *> tempPath(
        "tempPath",
        "Path to a tmp directory. "
        "Something like /tmp/ or c:\\temp\\, depending on platform..");


    parser.AddParameter(tempPath);

    int returnCode = 0;

    if (parser.TryParse(std::cout, argc, argv))
    {
        try
        {
            BitFunnel::ShardId shard = 0;
            double density = 0.1;
            double snr = 10.0;
            double adhocFrequency = 0.001;

            BitFunnel::BuildTermTable(tempPath,
                                      shard,
                                      density,
                                      snr,
                                      adhocFrequency);
            returnCode = 0;
        }
        catch (...)
        {
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
