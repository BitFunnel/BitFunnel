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


#include <algorithm>
#include <iostream>

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/Exceptions.h"
#include "BitFunnel/IFileManager.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IDocumentFrequencyTable.h"
#include "BitFunnel/Index/ITermToText.h"
#include "CmdLineParser/CmdLineParser.h"
#include "QueryGenerator.h"
#include "QueryLogBuilderTool.h"


namespace BitFunnel
{
    QueryLogBuilderTool::QueryLogBuilderTool(IFileSystem& fileSystem)
        : m_fileSystem(fileSystem)
    {
    }


    int QueryLogBuilderTool::Main(std::istream& /*input*/,
                                  std::ostream& output,
                                  int argc,
                                  char const *argv[])
    {
        CmdLine::CmdLineParser parser(
            "QueryLogBuilderTool",
            "Generate a query log from a DocumentFrequencyTable.");

        CmdLine::RequiredParameter<char const *> config(
            "config",
            "Path to configuration directory containing files generated "
            "by the 'BitFunnel statistics'.command.");

        CmdLine::RequiredParameter<int> queryCount(
            "queryCount",
            "Number of queries to generate.");

        CmdLine::RequiredParameter<double> m1(
            "m1",
            "Mean number of terms in a query.");

        CmdLine::RequiredParameter<double> s1(
            "s1",
            "Standard deviation for number of terms in a query.");


        parser.AddParameter(config);
        parser.AddParameter(queryCount);
        parser.AddParameter(m1);
        parser.AddParameter(s1);

        int returnCode = 1;

        if (parser.TryParse(output, argc, argv))
        {
            try
            {
                BuildQueryLog(output,
                              config,
                              queryCount,
                              m1,
                              s1);

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


    double Gaussian(double mean, double dev, double x)
    {
        static const double InvSqrt2Pi = 0.3989422804014327;
        double delta = (x - mean) / dev;

        return InvSqrt2Pi / dev * std::exp(-0.5 * delta * delta);
    }


    void QueryLogBuilderTool::BuildQueryLog(
        std::ostream& output,
        char const * configDirectory,
        size_t queryCount,
        double m1,
        double s1) const
    {
        output << "Loading document frequency table . . ." << std::endl;

        auto fileManager = Factories::CreateFileManager(configDirectory,
                                                        configDirectory,
                                                        configDirectory,
                                                        m_fileSystem);

        //
        // Generate the distribution of term counts.
        //

        std::vector<double> termCountDistribution;
        termCountDistribution.push_back(0);     // No empty queries.

        const size_t maxTermCount = 8;
        double cumulativeDistribution = 0;

        for (size_t i = 1; i <= maxTermCount; ++i)
        {
            double probability = Gaussian(m1, s1, static_cast<double>(i));
            std::cout << i << ": " << probability << std::endl;

            cumulativeDistribution += probability;
            termCountDistribution.push_back(probability);
        }

        // Normalize the distribution.
        for (size_t i = 1; i <= maxTermCount; ++i)
        {
            termCountDistribution[i] /= cumulativeDistribution;
        }

        //
        // Generate the queries.
        //

        const ShardId c_shardId = 0;

        auto dft(Factories::CreateDocumentFrequencyTable(
            *fileManager->DocFreqTable(c_shardId).OpenForRead()));

        auto terms(Factories::CreateTermToText(
            *fileManager->TermToText().OpenForRead()));


        QueryGenerator generator(*dft, *terms);

        std::vector<std::string> queries;
        queries.reserve(queryCount);

        for (size_t termCount = 0;
             termCount < termCountDistribution.size();
             ++termCount)
        {
            size_t count =
                static_cast<size_t>(queryCount * termCountDistribution[termCount]);
            for (size_t i = 0; i < count; ++i)
            {
                queries.push_back(generator.CreateOneQuery(termCount));
            }
        }

        //
        // Put queries in random order.
        //

        std::vector<size_t> permutation;
        permutation.reserve(queries.size());
        for (size_t i = 0; i < queries.size(); ++i)
        {
            permutation.push_back(i);
        }

        // TODO: Provide a generator so that we can ensure same results each run.
        std::random_shuffle(permutation.begin(), permutation.end());

        // Write out query log.
        {
            auto logFile = fileManager->QueryLog().OpenForWrite();
            for (size_t i = 0; i < permutation.size(); ++i)
            {
                *logFile << queries[permutation[i]] << std::endl;
            }
        }

        output << "Done." << std::endl;
    }
}
