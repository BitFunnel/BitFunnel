
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
        std::string const & filePath,
        IIndex& index)
      : m_filePath(filePath),
        m_index(index)
    {
        std::cout << "ChunkIngestor: filePath:" << m_filePath << std::endl;
        m_index.noop();

        // TODO: We should transition this to actually opening the files.
        // std::ifstream chunkFile;
        // chunkFile.open(filePath);

        // chunkFile.exceptions(std::ifstream::badbit);
        // if (!chunkFile.is_open()) {
        //     throw "MIke thought this was a good idea.";
        // }

        char const chunk[] =
            // First document
            "Title\0Dogs\0\0"
            "Body\0Dogs\0are\0man's\0best\0friend.\0\0"
            "\0"

            // Second document
            "Title\0Cat\0Facts\0\0"
            "Body\0The\0internet\0is\0made\0of\0cats.\0\0"
            "\0"

            // End of corpus
            "\0";

        ChunkReader(std::vector<char>(chunk, chunk + sizeof(chunk)), *this);
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
