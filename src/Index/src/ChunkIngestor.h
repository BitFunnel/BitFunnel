#pragma once

#include <memory>                       // std::unqiue_ptr member.
#include <vector>                       // std::vector member.

#include "BitFunnel/Index/IDocument.h"  // std::unique_ptr<IDocument>.
#include "BitFunnel/NonCopyable.h"      // Inherits from NonCopyable.
#include "ChunkReader.h"                // Inherits from ChunkReader::IEvents.


namespace BitFunnel
{
    class IDocumentFactory
    {
    };
}


namespace BitFunnel
{
    class IConfiguration;
    class IIngestor;

    // DESIGN NOTE: Consider adding a document factory parameter to the
    // constructor.
    class ChunkIngestor : public NonCopyable, public ChunkReader::IEvents
    {
    public:
        // TODO: We need to implement IDocumentFactory before this make sense.
        // ChunkIngestor(std::string const & filePath, IIndex& index,
        //               IDocumentFactory& factory);
        ChunkIngestor(std::vector<char> const& chunkData,
                      IConfiguration const & configuration,
                      IIngestor& ingestor);

        //
        // ChunkReader::IEvents methods.
        //
        virtual void OnFileEnter() override;
        virtual void OnDocumentEnter(DocId id) override;
        virtual void OnStreamEnter(char const * name) override;
        virtual void OnTerm(char const * term) override;
        virtual void OnStreamExit() override;
        virtual void OnDocumentExit() override;
        virtual void OnFileExit() override;

    private:
        //
        // Constructor parameters
        //
        IConfiguration const & m_config;
        IIngestor& m_ingestor;

        //
        // Other members
        //
        std::vector<char> const& m_chunkData;
        std::unique_ptr<IDocument> m_currentDocument;
    };
}
