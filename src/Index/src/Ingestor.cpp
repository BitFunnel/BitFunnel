// The MIT License (MIT)

// Copyright (c) 2016, Microsoft

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <iostream>     // TODO: Remove this temporary header.
#include <memory>

#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IDocument.h"
#include "BitFunnel/Utilities/Factories.h"
#include "DocumentHandleInternal.h"
#include "Ingestor.h"
#include "IRecycler.h"
#include "ISliceBufferAllocator.h"


namespace BitFunnel
{
    std::unique_ptr<IIngestor>
    Factories::CreateIngestor(IDocumentDataSchema const & docDataSchema,
                              IRecycler& recycler,
                              ITermTable const & termTable,
                              ISliceBufferAllocator& sliceBufferAllocator)
    {
        return std::unique_ptr<IIngestor>(new Ingestor(docDataSchema,
                                                       recycler,
                                                       termTable,
                                                       sliceBufferAllocator));
    }

    Ingestor::Ingestor(IDocumentDataSchema const & docDataSchema,
                       IRecycler& recycler,
                       ITermTable const & termTable,
                       ISliceBufferAllocator& sliceBufferAllocator)
        : m_recycler(recycler),
          m_documentCount(0),
          m_tokenManager(Factories::CreateTokenManager()),
          m_sliceBufferAllocator(sliceBufferAllocator)
    {
        // Initialize histogram and frequency tables here.
        // TODO: make order of parameters in Shard as similar as possible to
        // order of parameters in Ingestor.
        m_shards.push_back(
            std::unique_ptr<Shard>(
                new Shard(*this,
                          123,
                          termTable,
                          docDataSchema,
                          m_sliceBufferAllocator,
                          m_sliceBufferAllocator.GetSliceBufferSize())));
    }

    void Ingestor::PrintStatistics() const
    {
        std::cout << "Document count: " << m_documentCount << std::endl;
        std::cout << "Term count: " << m_postingsCount.m_totalCount << std::endl;

        for (auto it = m_shards.begin(); it != m_shards.end(); ++it)
        {
            (*it)->TemporaryPrintFrequencies(std::cout);
        }

        std::cout << "Posting count histogram" << std::endl;

        m_postingsCount.Write(std::cout);
    }



    void Ingestor::Add(DocId /*id*/, IDocument const & document)
    {
        ++m_documentCount;

        // Add postingCount to the DocumentLengthHistogram
        m_postingsCount.AddDocument(document.GetPostingCount());

        DocumentHandleInternal handle = m_shards[0]->AllocateDocument();
        document.Ingest(handle);
    }


    IRecycler& Ingestor::GetRecycler() const
    {
        return m_recycler;
    }


    size_t Ingestor::GetShardCount() const
    {
        return m_shards.size();
    }


    Shard& Ingestor::GetShard(size_t shard) const
    {
        return *(m_shards[shard]);
    }


    ITokenManager& Ingestor::GetTokenManager() const
    {
        return *m_tokenManager;
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
        m_tokenManager->Shutdown();
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
