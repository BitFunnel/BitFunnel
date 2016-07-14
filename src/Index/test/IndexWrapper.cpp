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


#include <sstream>

#include "BitFunnelAllocatorInterfaces/IAllocatorFactory.h"
#include "BitFunnel/Factories.h"
#include "BitFunnel/ITermTable.h"
#include "BitFunnel/IShardDefinition.h"
#include "BitFunnel/PostingDocumentBlobs.h"
#include "DocumentDataSchema.h"
#include "IndexWrapper.h"
#include "PrivateHeapAllocator.h"
#include "SingleShardDefinition.h"
// #include "SuiteCpp/UnitTest.h"


namespace
{
    const bool c_performOfflinePerDocumentDataIngestion = true;
    const bool c_performFalsePositiveFiltering = true;
    const unsigned c_maxGramSizeForBloomFilter = 2;
    const size_t c_hashFunctionCountForBloomFilter = 2;
    const size_t c_bitCountPerTermForBloomFilter = 3;
    const size_t c_offlinePerDocumentDataAlignmentSizeInBytes = 4 * 1024;
}

namespace BitFunnel
{
    // This value is used to pre-allocate space in DocumentMap in Index.
    // It can be made reasonably large.
    static const DocIndex c_maxIndexCapacity = 100 * 1000;


    IndexWrapper::IndexWrapper(DocIndex sliceCapacity,
                               std::shared_ptr<ITermTable const> const & termTable,
                               IDocumentDataSchema const & docDataSchema,
                               unsigned blockAllocatorBlockCount)
        : m_sliceBufferAllocator(CreateSliceBufferAllocator(sliceCapacity,
                                                            docDataSchema,
                                                            *termTable,
                                                            blockAllocatorBlockCount)),
          m_offlinePerDocumentDataAllocatorFactory(new PrivateHeapAllocatorFactory())
    {
        Initialize(docDataSchema, termTable, sliceCapacity);
    }


    IndexWrapper::IndexWrapper(DocIndex sliceCapacity,
                               std::shared_ptr<ITermTable const> const & termTable,
                               IDocumentDataSchema const & docDataSchema,
                               FixedSizeBlobId offlinePerDocumentDataBlobId,
                               unsigned blockAllocatorBlockCount)
        : m_sliceBufferAllocator(CreateSliceBufferAllocator(sliceCapacity,
                                                            docDataSchema,
                                                            *termTable,
                                                            blockAllocatorBlockCount)),
          m_offlinePerDocumentDataConfig(c_performOfflinePerDocumentDataIngestion,
                                         c_performFalsePositiveFiltering,
                                         offlinePerDocumentDataBlobId,
                                         c_maxGramSizeForBloomFilter,
                                         c_bitCountPerTermForBloomFilter,
                                         c_hashFunctionCountForBloomFilter,
                                         c_offlinePerDocumentDataAlignmentSizeInBytes),
          m_offlinePerDocumentDataAllocatorFactory(new PrivateHeapAllocatorFactory())
    {
        Initialize(docDataSchema, termTable, sliceCapacity);
    }


    IndexWrapper::IndexWrapper(DocIndex sliceCapacity,
                               std::shared_ptr<ITermTable const> const & termTable,
                               IDocumentDataSchema const & docDataSchema,
                               std::unique_ptr<ISliceBufferAllocator>& sliceBufferAllocator)
        : m_sliceBufferAllocator(sliceBufferAllocator.release()),
          m_offlinePerDocumentDataAllocatorFactory(new PrivateHeapAllocatorFactory())
    {
        Initialize(docDataSchema, termTable, sliceCapacity);
    }


    IndexWrapper::~IndexWrapper()
    {
        m_index->Shutdown();
    }


    IngestionIndex& IndexWrapper::GetIndex()
    {
        return *m_index;
    }


    Shard& IndexWrapper::GetShard()
    {
        return m_index->GetShard(0);
    }


    ISliceBufferAllocator& IndexWrapper::GetSliceBufferAllocator()
    {
        return *m_sliceBufferAllocator;
    }


    size_t IndexWrapper::GetSliceBufferSize() const
    {
        return m_sliceBufferAllocator->GetSliceBufferSize();
    }


    void IndexWrapper::Initialize(IDocumentDataSchema const & docDataSchema,
                                  std::shared_ptr<ITermTable const> const & termTable,
                                  DocIndex sliceCapacity)
    {
        m_termTables.reset(new SingleShardTermTableCollection(termTable, CreateSingleShardDefinition(sliceCapacity * 10)));
        m_files.reset(Factories::CreateFileManager("", "", "", "", ""));

        m_index.reset(new IngestionIndex(docDataSchema,
                                         *m_termTables,
                                         *m_files,
                                         *m_sliceBufferAllocator,
                                         c_maxIndexCapacity,
                                         m_recycler,
                                         m_offlinePerDocumentDataConfig,
                                         *m_offlinePerDocumentDataAllocatorFactory));
    }


    std::unique_ptr<SliceBufferAllocator>
    IndexWrapper::CreateSliceBufferAllocator(DocIndex sliceCapacity,
                                             IDocumentDataSchema const & docDataSchema,
                                             ITermTable const & termTable,
                                             unsigned blockAllocatorBlockCount)
    {
        return std::make_unique<SliceBufferAllocator>(Shard::InitializeDescriptors(nullptr,
                                                                                   sliceCapacity,
                                                                                   docDataSchema,
                                                                                   termTable),
                                                      blockAllocatorBlockCount);
    }
}
