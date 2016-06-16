
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "BitFunnel/Index/IIndex.h"
#include "BitFunnel/Index/IngestChunks.h"
#include "CmdLineParser/CmdLineParser.h"

static std::vector<std::string> readLines(char const * fileName)
{
    std::ifstream file(fileName);

    std::vector<std::string> lines;
    std::string line;
    while(std::getline(file, line)) {
        lines.push_back(std::move(line));
    }

    return lines;
}


static void loadAndIngestChunkList(char const * chunkListFileName)
{
    std::cout << "Loading chunk list file '" << chunkListFileName << "'"
        << std::endl;
    std::vector<std::string> filePaths = readLines(chunkListFileName);

    BitFunnel::IIndex index;
    BitFunnel::IngestChunks(filePaths, index, 2);
}


int main(int argc, char** argv)
{
    CmdLine::CmdLineParser parser(
        "StatisticsBuilder",
        "Ingest documents and compute statistics about them.");

    CmdLine::RequiredParameter<char const *> chunkListFileName(
        "chunkListFileName",
        "Path to the file to convert to a vector.");

    parser.AddParameter(chunkListFileName);

    if (!parser.TryParse(std::cout, argc, argv))
    {
        return 0;
    }

    loadAndIngestChunkList(chunkListFileName);

    return 0;
}
