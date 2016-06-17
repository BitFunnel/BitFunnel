#include <memory>
#include <stdexcept>

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
    }


    void Ingestor::Add(DocId /*id*/, IDocument const & document)
    {
        ++m_documentCount;
        m_termCount += document.GetPostingCount();

        DocumentHandleInternal handle;
        document.Ingest(handle);
    }


    bool Ingestor::Delete(DocId /*id*/)
    {
        throw std::runtime_error("No implemented.");
    }


    void Ingestor::AssertFact(DocId /*id*/, FactHandle /*fact*/, bool /*value*/)
    {
        throw std::runtime_error("No implemented.");
    }


    bool Ingestor::Contains(DocId /*id*/) const
    {
        throw std::runtime_error("No implemented.");
    }


    size_t Ingestor::GetUsedCapacityInBytes() const
    {
        throw std::runtime_error("No implemented.");
    }


    void Ingestor::Shutdown()
    {
        throw std::runtime_error("No implemented.");
    }


    void Ingestor::OpenGroup(GroupId /*groupId*/)
    {
        throw std::runtime_error("No implemented.");
    }


    void Ingestor::CloseGroup()
    {
        throw std::runtime_error("No implemented.");
    }


    void Ingestor::ExpireGroup(GroupId /*groupId*/)
    {
        throw std::runtime_error("No implemented.");
    }
}
