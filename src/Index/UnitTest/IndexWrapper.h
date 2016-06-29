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
        std::unique_ptr<Allocators::IAllocatorFactory> m_offlinePerDocumentDataAllocatorFactory;
        OfflinePerDocumentDataConfig m_offlinePerDocumentDataConfig;
        std::unique_ptr<IngestionIndex> m_index;

        const std::unique_ptr<IBlockAllocator> m_blockAllocator;
    };
}
