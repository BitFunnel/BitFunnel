
#include "ChunkEnumerator.h"
#include "ChunkTaskProcessor.h"

namespace BitFunnel
{
    ChunkEnumerator::ChunkEnumerator(
        std::vector<std::string> const & filePaths,
        IIngestor& ingestor,
        size_t threadCount)
      : m_ingestor(ingestor)
    {
        std::vector<ITaskProcessor*> processors;
        for (size_t i = 0; i < threadCount; ++i) {
            processors.push_back(new ChunkTaskProcessor(filePaths, m_ingestor));
        }

        // TODO: The task distributor factory needs to be moved or created before
        // we can do use this.
        // m_distributor = Factories::CreateTaskDistributor(processors,
        //                                                  filePaths.size());
        // distributor.WaitForCompletion();

        // TODO: Destruct the processors when we're done with them.

        for (size_t i = 0; i < filePaths.size(); ++i) {
            processors[0]->ProcessTask(i);
        }
    }

    void ChunkEnumerator::WaitForCompletion() const
    {
    }
}