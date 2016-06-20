#include <iostream>     // TODO: Remove this temporary header.

#include "BitFunnel/Exceptions.h"
#include "Shard.h"
#include "Term.h"       // TODO: Remove this temporary include.


namespace BitFunnel
{
    Shard::Shard(IIngestor& ingestor)
        : m_ingestor(ingestor),
          m_slice(new Slice(*this))
    {
        m_activeSlice = m_slice.get();
    }


    void Shard::TemporaryAddPosting(Term const & term, DocIndex index)
    {
        std::cout << "  " << index << ": ";
        term.Print(std::cout);
        std::cout << std::endl;
    }

    DocumentHandleInternal Shard::AllocateDocument()
    {
        DocIndex index;
        if (!m_activeSlice->TryAllocateDocument(index))
        {
            throw FatalError("In this temporary code, TryAllocateDocument() should always succeed.");
        }
        return DocumentHandleInternal(m_activeSlice, index);
    }


    IIngestor& Shard::GetIndex() const
    {
        return m_ingestor;
    }
}
