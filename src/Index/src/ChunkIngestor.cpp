#include <iostream>     // TODO: Remove this temporary header.

#include "BitFunnel/Index/IIngestor.h"
#include "ChunkIngestor.h"
#include "Document.h"


namespace BitFunnel
{
    ChunkIngestor::ChunkIngestor(
        std::vector<char> const& chunkData,
        IConfiguration const & config,
        IIngestor& ingestor)
      : m_config(config),
        m_ingestor(ingestor),
        m_chunkData(chunkData)
    {
        ChunkReader(m_chunkData, *this);
    }


    void ChunkIngestor::OnFileEnter()
    {
        std::cout << "ChunkIngestor::OnFileEnter" << std::endl;
    }


    void ChunkIngestor::OnDocumentEnter(DocId id)
    {
        std::cout << "ChunkIngestor::OnDocumentEnter: id:" << id << std::endl;
        m_currentDocument.reset(new Document(m_config));
    }


    void ChunkIngestor::OnStreamEnter(char const * name)
    {
        std::cout << "ChunkIngestor::OnStreamEnter: name:" << name << std::endl;
        m_currentDocument->OpenStream(name);
    }


    void ChunkIngestor::OnTerm(char const * term)
    {
        std::cout << "ChunkIngestor::OnTerm: term:" << term << std::endl;
        m_currentDocument->AddTerm(term);
    }


    void ChunkIngestor::OnStreamExit()
    {
        std::cout << "ChunkIngestor::OnStreamExit" << std::endl;
        m_currentDocument->CloseStream();
    }


    void ChunkIngestor::OnDocumentExit()
    {
        std::cout << "ChunkIngestor::OnDocumentExit" << std::endl;
        m_ingestor.Add(0, *m_currentDocument);
        m_currentDocument.reset(nullptr);
    }


    void ChunkIngestor::OnFileExit()
    {
        std::cout << "ChunkIngestor::OnFileExit" << std::endl;
    }
}
