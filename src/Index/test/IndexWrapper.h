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


#pragma once

#include <memory>

#include "BitFunnel/IFileManager.h"
#include "IngestionIndex.h"
#include "SingleShardTermTableCollection.h"
#include "SliceBufferAllocator.h"
#include "SynchronousRecycler.h"


namespace BitFunnel
{
    namespace Allocators
    {
        class IAllocatorFactory;
    }
    class IDocumentDataSchema;
    class IShardDefinition;
    class PostingDocumentBlobs;

    // This is a factory that creates an Index, plus things an Index depends on. The
    // intent is to use this for tests so that tests don't need a lot of scaffolding
    // to create an Index. TODO: port this to Ingestor.
    class IndexWrapper
    {
    public:
        IndexWrapper(DocIndex sliceCapacity,
                     std::shared_ptr<ITermTable const> const & termTable,
                     IDocumentDataSchema const & docDataSchema,
                     unsigned blockAllocatorBlockCount);

        // Constructor which enables false positive filtering.
        IndexWrapper(DocIndex sliceCapacity,
                     std::shared_ptr<ITermTable const> const & termTable,
                     IDocumentDataSchema const & docDataSchema,
                     FixedSizeBlobId offlinePerDocumentDataBlobId,
                     unsigned blockAllocatorBlockCount);

        // Constructor which takes a specific ISliceBufferAllocator.
        // IndexWrapper takes ownership of the allocator.
        IndexWrapper(DocIndex sliceCapacity,
                     std::shared_ptr<ITermTable const> const & termTable,
                     IDocumentDataSchema const & docDataSchema,
                     std::unique_ptr<ISliceBufferAllocator>& sliceBufferAllocator);

        ~IndexWrapper();

        IngestionIndex& GetIndex();
        Shard& GetShard();
        ISliceBufferAllocator& GetSliceBufferAllocator();
        size_t GetSliceBufferSize() const;

    private:
        void Initialize(IDocumentDataSchema const & docDataSchema,
                        std::shared_ptr<ITermTable const> const & termTable,
                        DocIndex sliceCapacity);

        std::unique_ptr<SliceBufferAllocator> CreateSliceBufferAllocator(DocIndex sliceCapacity,
                                                                         IDocumentDataSchema const & docDataSchema,
                                                                         ITermTable const & termTable,
                                                                         unsigned blockAllocatorBlockCount);

        // WARNING: Initialization order dependency on the following members.
        std::unique_ptr<IFileManager> m_files;
        std::unique_ptr<ISliceBufferAllocator> m_sliceBufferAllocator;
        std::unique_ptr<SingleShardTermTableCollection> m_termTables;
        SynchronousRecycler m_recycler;
        std::unique_ptr<IAllocatorFactory> m_offlinePerDocumentDataAllocatorFactory;
        OfflinePerDocumentDataConfig m_offlinePerDocumentDataConfig;
        std::unique_ptr<IngestionIndex> m_index;

        const std::unique_ptr<IBlockAllocator> m_blockAllocator;
    };
}
