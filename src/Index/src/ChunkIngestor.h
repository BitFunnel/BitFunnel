#pragma once

#include "BitFunnel/Index/DocumentHandle.h"
#include "BitFunnel/NonCopyable.h"
#include "ChunkReader.h"

namespace BitFunnel
{
    class IDocumentFactory
    {
    };
}


namespace BitFunnel
{
    class IIngestor;

    // DESIGN NOTE: Consider adding a document factory parameter to the
    // constructor.
    class ChunkIngestor : public NonCopyable, public ChunkReader::IEvents
    {
    public:
        // TODO: We need to implement IDocumentFactory before this make sense.
        // ChunkIngestor(std::string const & filePath, IIndex& index,
        //               IDocumentFactory& factory);
        ChunkIngestor(std::vector<char> const& chunkData, IIngestor& ingestor);

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
        std::vector<char> const& m_chunkData;
        IIngestor& m_ingestor;
    };
}
