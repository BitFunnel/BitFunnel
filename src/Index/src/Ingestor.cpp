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
#include "LoggerInterfaces/Logging.h"


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
          m_documentCount(0),   // TODO: This member is now redundant (with m_documentMap).
          m_documentMap(new DocumentMap()),
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
    }


    void Ingestor::WriteDocumentFrequencyTable(std::ostream& out) const
    {
        // TODO: Either write one file per shard, or move term
        // frequency table up to ingestor.
        m_shards[0]->TemporaryWriteDocumentFrequencyTable(out);
    }


    void Ingestor::WriteDocumentLengthHistogram(std::ostream & out) const
    {
        m_postingsCount.Write(out);
    }


    void Ingestor::WriteCumulativePostingCounts(std::ostream & out) const
    {
        // TODO: Either write one file per shard, or move term
        // frequency table up to ingestor.
        m_shards[0]->TemporaryWriteCumulativePostingCounts(out);
    }


    void Ingestor::Add(DocId id, IDocument const & document)
    {
        ++m_documentCount;

        // Add postingCount to the DocumentLengthHistogram
//        std::cout << "DocId: " << id << ": " << document.GetPostingCount() << std::endl;
        m_postingsCount.AddDocument(document.GetPostingCount());

        DocumentHandleInternal handle = m_shards[0]->AllocateDocument(id);
        document.Ingest(handle);


        // TODO: REVIEW: Why are Activate() and CommitDocument() separate operations?
        handle.Activate();
        handle.GetSlice()->CommitDocument();

        // TODO: schedule for backup if Slice is full.
        // Consider if Slice::CommitDocument itself may schedule a backup when full.

        try
        {
            m_documentMap->Add(handle);
        }
        catch (...)
        {
            try
            {
                handle.Expire();
            }
            catch (...)
            {
                LogB(Logging::Error,
                     "Ingestor::Add",
                     "Error while cleaning up after AddDocument operation failed.",
                     "");
            }

            // Re-throw the original exception back to the caller.
            throw;
        }
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


    bool Ingestor::Delete(DocId id)
    {
        const Token token = m_tokenManager->RequestToken();

        // Protecting from concurrent Delete operations. Even though individual
        // function calls here are thread-safe, Delete on the same value of DocId
        // is not, since it modifies the counters of the expired documents in the
        // Slice.
        std::lock_guard<std::mutex> lock(m_deleteDocumentLock);

        bool isFound;
        DocumentHandleInternal location = m_documentMap->Find(id, isFound);

        if (isFound)
        {
            m_documentMap->Delete(id);
            location.Expire();
        }

        // In a case of documents deletes, a missing entry should not be treated
        // as an error. This is to accommodate soft-deleting a large number of
        // documents where only the range of IDs is known, but not the exact
        // values.

        return isFound;
    }


    void Ingestor::AssertFact(DocId /*id*/, FactHandle /*fact*/, bool /*value*/)
    {
        throw NotImplemented();
    }


    bool Ingestor::Contains(DocId id) const
    {
        bool isFound;
        m_documentMap->Find(id, isFound);

        return isFound;
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
