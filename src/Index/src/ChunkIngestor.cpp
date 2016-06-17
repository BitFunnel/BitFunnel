
#include <fstream>
#include <iostream>
#include <sstream>

#include "ChunkEnumerator.h"
#include "ChunkIngestor.h"


namespace BitFunnel
{
    ChunkIngestor::ChunkIngestor(
        // TODO: We need to implement IDocumentFactory before this make sense.
        // std::string const & filePath,
        // IIndex& index,
        // IDocumentFactory& factory)
        std::vector<char> const& chunkData,
        IIngestor& ingestor)
      : m_chunkData(chunkData),
        m_ingestor(ingestor)
    {

        // TODO: We should transition this to actually opening the files.
        // std::ifstream chunkFile;
        // chunkFile.open(filePath);

        // chunkFile.exceptions(std::ifstream::badbit);
        // if (!chunkFile.is_open()) {
        //     throw "MIke thought this was a good idea.";
        // }

        ChunkReader(m_chunkData, *this);
    }

    void ChunkIngestor::OnFileEnter()
    {
        std::cout << "ChunkIngestor::OnFileEnter" << std::endl;
    }

    void ChunkIngestor::OnDocumentEnter(DocId id)
    {
        std::cout << "ChunkIngestor::OnDocumentEnter: id:" << id << std::endl;
    }

    void ChunkIngestor::OnStreamEnter(char const * name)
    {
        std::cout << "ChunkIngestor::OnStreamEnter: name:" << name << std::endl;
    }

    void ChunkIngestor::OnTerm(char const * term)
    {
        std::cout << "ChunkIngestor::OnTerm: term:" << term << std::endl;
    }

    void ChunkIngestor::OnStreamExit()
    {
        std::cout << "ChunkIngestor::OnStreamExit" << std::endl;
    }

    void ChunkIngestor::OnDocumentExit()
    {
        std::cout << "ChunkIngestor::OnDocumentExit" << std::endl;
    }

    void ChunkIngestor::OnFileExit()
    {
        std::cout << "ChunkIngestor::OnFileExit" << std::endl;
    }
}
