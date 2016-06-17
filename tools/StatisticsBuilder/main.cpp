#include <fstream>
#include <iostream>
#include <vector>

#include "BitFunnel/Index/IIndex.h"
#include "BitFunnel/Index/IngestChunks.h"
#include "CmdLineParser/CmdLineParser.h"


// Returns a vector with one entry for each line in the file.
static std::vector<std::string> ReadLines(char const * fileName)
{
    std::ifstream file(fileName);

    std::vector<std::string> lines;
    std::string line;
    while(std::getline(file, line)) {
        lines.push_back(std::move(line));
    }

    return lines;
}


static void LoadAndIngestChunkList(char const * chunkListFileName)
{
    std::cout << "Loading chunk list file '" << chunkListFileName << "'"
        << std::endl;
    std::vector<std::string> filePaths = ReadLines(chunkListFileName);

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
        "Path to a file containing the paths to the chunk files to be ingested. "
        "One chunk file per line. Paths are relative to working directory.");

    parser.AddParameter(chunkListFileName);

    int returnCode = 0;

    if (parser.TryParse(std::cout, argc, argv))
    {
        try
        {
            LoadAndIngestChunkList(chunkListFileName);
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
