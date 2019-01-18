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

#include <iostream>  // TODO: remove.

#include "BitFunnel/Configuration/IShardDefinition.h"
#include "BitFunnel/Exceptions.h"
#include "BitFunnel/IFileManager.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IDocument.h"
#include "BitFunnel/Index/IRecycler.h"
#include "BitFunnel/Index/ISliceBufferAllocator.h"
#include "BitFunnel/Index/ITermTableCollection.h"
#include "BitFunnel/Utilities/Factories.h"
#include "BitFunnel/Utilities/StreamUtilities.h"
#include "DocumentHandleInternal.h"
#include "Ingestor.h"
#include "LoggerInterfaces/Logging.h"
#include "TermToText.h"


namespace BitFunnel
{
    std::unique_ptr<IIngestor>
    Factories::CreateIngestor(IDocumentDataSchema const & docDataSchema,
                              IRecycler& recycler,
                              ITermTableCollection const & termTables,
                              IShardDefinition const & shardDefinition,
                              ISliceBufferAllocator& sliceBufferAllocator)
    {
        return std::unique_ptr<IIngestor>(new Ingestor(docDataSchema,
                                                       recycler,
                                                       termTables,
                                                       shardDefinition,
                                                       sliceBufferAllocator));
    }


    Ingestor::Ingestor(IDocumentDataSchema const & docDataSchema,
                       IRecycler& recycler,
                       ITermTableCollection const & termTables,
                       IShardDefinition const & shardDefinition,
                       ISliceBufferAllocator& sliceBufferAllocator)
        : m_recycler(recycler),
          m_shardDefinition(shardDefinition),
          // TODO: This member is now redundant (with m_documentMap).
          // But see issue 389. Because of that issue, m_documentCount is not
          // always equal to m_documentMap.size().
          m_documentCount(0),
          m_totalSourceByteSize(0),
          m_documentMap(new DocumentMap()),
          m_documentCache(new DocumentCache()),
          m_tokenManager(Factories::CreateTokenManager()),
          m_sliceBufferAllocator(sliceBufferAllocator)
    {
        // Create shards based on shard definition in m_shardDefinition..
        for (ShardId shardId = 0; shardId < m_shardDefinition.GetShardCount(); ++shardId)
        {
            std::cout
                << "Creating shard: " << shardId;
            if (shardId + 1 < m_shardDefinition.GetShardCount())
            {
                std::cout << " (" << m_shardDefinition.GetMaxPostingCount(shardId) << " postings)" << std::endl;
            }
            else
            {
                std::cout << " (larger)" << std::endl;
            }

            m_shards.push_back(
                std::unique_ptr<Shard>(
                    new Shard(shardId,
                              GetRecycler(),
                              GetTokenManager(),
                              termTables.GetTermTable(shardId),
                              docDataSchema,
                              m_sliceBufferAllocator,
                              m_sliceBufferAllocator.GetSliceBufferSize())));
        }
    }


    Ingestor::~Ingestor()
    {
        Shutdown();
    }


    void Ingestor::PrintStatistics(std::ostream& out,
                                   double time) const
    {
        out << "Shard count:" << m_shards.size() << std::endl
            << "Document count: " << m_documentCount << std::endl
            << "Bytes/Document: "
            << static_cast<double>(m_totalSourceByteSize) / m_documentCount
            << std::endl
            << "Total bytes read: " << m_totalSourceByteSize << std::endl
            << "Posting count: " << m_histogram.GetPostingCount() << std::endl;

        if (time > 0)
        {
            out << "Total ingestion time: " << time << std::endl;
            out << "Bytes/second: " << m_totalSourceByteSize / time << std::endl;
        }

        out << std::endl;

        // TODO: print out term count? Not sure how to do this since they are spread across shards.
    }


    void Ingestor::WriteStatistics(IFileManager & fileManager,
                                   ITermToText const * termToText) const
    {
        if (termToText != nullptr)
        {
            auto out = fileManager.TermToText().OpenForWrite();
            termToText->Write(*out);
        }

        {
            auto out = fileManager.DocumentHistogram().OpenForWrite();
            m_histogram.Write(*out);
        }

        for (size_t shard = 0; shard < m_shards.size(); ++shard)
        {
            {
                auto out = fileManager.CumulativeTermCounts(shard).OpenForWrite();
                m_shards[shard]->TemporaryWriteCumulativeTermCounts(*out);
            }
            {
                auto out = fileManager.DocFreqTable(shard).OpenForWrite();
                m_shards[shard]->TemporaryWriteDocumentFrequencyTable(*out, termToText);
            }
        }
    }


    void Ingestor::TemporaryReadAllSlices(IFileManager& fileManager)
    {
        // Recover ingestor-wide values from IndexSliceMain file
        // and make sure saved slices are formatted consistently with current index
        // Note:  PostingCount is neither saved nor recovered
        auto sliceFileMain = fileManager.IndexSliceMain();
        auto input = sliceFileMain.OpenForRead();
        m_documentCount = StreamUtilities::ReadField<size_t>(*input);
        m_totalSourceByteSize = StreamUtilities::ReadField<size_t>(*input);
        auto shardSize = StreamUtilities::ReadField<size_t>(*input);
        auto sliceBufferSize = StreamUtilities::ReadField<size_t>(*input);
        if (shardSize != m_shards.size() || sliceBufferSize != m_sliceBufferAllocator.GetSliceBufferSize())
        {
            RecoverableError error("Ingestor::TemporaryReadAllSlices(): Saved slices don't match index format.");
            throw error;
        }

        // Load each shard's slices
        for (size_t i = 0; i < m_shards.size(); ++i)
        {
            auto nbrSlices = StreamUtilities::ReadField<size_t>(*input);
            m_shards[i]->TemporaryReadAllSlices(fileManager, nbrSlices);
        }
    }


    void Ingestor::TemporaryWriteAllSlices(IFileManager& fileManager) const
    {
        // Write out IndexSliceMain file with ingestor-wide values
        auto sliceFileMain = fileManager.IndexSliceMain();
        auto output = sliceFileMain.OpenForWrite();
        StreamUtilities::WriteField<size_t>(*output, m_documentCount);
        StreamUtilities::WriteField<size_t>(*output, m_totalSourceByteSize);
        StreamUtilities::WriteField<size_t>(*output, m_shards.size());
        StreamUtilities::WriteField<size_t>(*output, m_sliceBufferAllocator.GetSliceBufferSize());

        // Save each shard's slices
        for (size_t i = 0; i < m_shards.size(); ++i)
        {
            StreamUtilities::WriteField<size_t>(*output, m_shards[i]->GetSliceBuffers().size());
            m_shards[i]->TemporaryWriteAllSlices(fileManager);
        }
    }


    IDocumentCache & Ingestor::GetDocumentCache() const
    {
        return *m_documentCache;
    }


    void Ingestor::Add(DocId id, IDocument const & document)
    {
        ++m_documentCount;
        m_totalSourceByteSize += document.GetSourceByteSize();

        // Add postingCount to the DocumentHistogramBuilder
        m_histogram.AddDocument(document.GetPostingCount());

        // Choose correct shard and then allocate handle.
        ShardId shardId = m_shardDefinition.GetShard(document.GetPostingCount());
        DocumentHandleInternal handle = m_shards[shardId]->AllocateDocument(id);

        // std::cout
        //    << "IIngestor::Add("
        //    << id << "):"
        //    << " postingCount: " << document.GetPostingCount()
        //    << " shardId: " << shardId
        //    << std::endl;

        document.Ingest(handle);


        // TODO: REVIEW: Why are Activate() and CommitDocument() separate operations?
        handle.Activate();
        handle.GetSlice().CommitDocument();

        // TODO: schedule for backup if Slice is full.
        // Consider if Slice::CommitDocument itself may schedule a backup when full.

        try
        {
            m_documentMap->Add(handle);
            // TODO: Remove this debugging code. Related to issue 389.
            //if (m_documentMap->size() != m_documentCount)
            //{
            //    std::cout
            //        << "  Mismatch on docId=" << id
            //        << ": m_documentMap->size()=" << m_documentMap->size()
            //        << ", m_documentCount = " << m_documentCount
            //        << std::endl;
            //}
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


    IShard& Ingestor::GetShard(size_t shard) const
    {
        return *(m_shards[shard]);
    }


    IShardDefinition const & Ingestor::GetShardDefinition() const
    {
        return m_shardDefinition;
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


    DocumentHandle Ingestor::GetHandle(DocId id) const
    {
        bool isFound;
        auto handle = m_documentMap->Find(id, isFound);

        if (!isFound)
        {
            RecoverableError error("Ingestor::GetHandle(): DocId not found.");
            throw error;
        }

        return handle;
    }


    size_t Ingestor::GetDocumentCount() const
    {
        return m_documentCount;
//        return m_documentMap->size();
    }


    size_t Ingestor::GetUsedCapacityInBytes() const
    {
        throw NotImplemented();
    }


    size_t Ingestor::GetTotalSouceBytesIngested() const
    {
        return m_totalSourceByteSize;
    }


    size_t Ingestor::GetPostingCount() const
    {
        return m_histogram.GetPostingCount();
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
