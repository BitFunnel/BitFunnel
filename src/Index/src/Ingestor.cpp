#include <iostream>     // TODO: Remove this temporary header.
#include <memory>

#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IDocument.h"
#include "DocumentHandleInternal.h"
#include "Ingestor.h"


namespace BitFunnel
{
    std::unique_ptr<IIngestor> Factories::CreateIngestor()
    {
        return std::unique_ptr<IIngestor>(new Ingestor());
    }


    Ingestor::Ingestor()
        : m_termCount(0),
          m_documentCount(0)
    {
        // Initialize histogram and frequency tables here.
        m_shards.push_back(std::unique_ptr<Shard>(new Shard(*this)));
    }


    void Ingestor::PrintStatistics() const
    {
        std::cout << "Document count: " << m_documentCount << std::endl;
        std::cout << "Term count: " << m_termCount << std::endl;
    }



    void Ingestor::Add(DocId /*id*/, IDocument const & document)
    {
        ++m_documentCount;
        m_termCount += document.GetPostingCount();

        DocumentHandleInternal handle = m_shards[0]->AllocateDocument();
        document.Ingest(handle);
    }


    bool Ingestor::Delete(DocId /*id*/)
    {
        throw NotImplemented();
    }


    void Ingestor::AssertFact(DocId /*id*/, FactHandle /*fact*/, bool /*value*/)
    {
        throw NotImplemented();
    }


    bool Ingestor::Contains(DocId /*id*/) const
    {
        throw NotImplemented();
    }


    size_t Ingestor::GetUsedCapacityInBytes() const
    {
        throw NotImplemented();
    }


    void Ingestor::Shutdown()
    {
        throw NotImplemented();
    }


    void Ingestor::OpenGroup(GroupId /*groupId*/)
    {
        throw NotImplemented();
    }


    void Ingestor::CloseGroup()
    {
        throw NotImplemented();
    }


    void Ingestor::ExpireGroup(GroupId /*groupId*/)
    {
        throw NotImplemented();
    }
}
