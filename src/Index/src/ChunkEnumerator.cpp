
#include "BitFunnel/Utilities/ITaskDistributor.h"
#include "BitFunnel/Utilities/Factories.h"
#include "ChunkEnumerator.h"
#include "ChunkTaskProcessor.h"


namespace BitFunnel
{
    ChunkEnumerator::ChunkEnumerator(
        std::vector<std::string> const & filePaths,
        IConfiguration const & config,
        IIngestor& ingestor,
        size_t threadCount)
    {
        std::vector<std::unique_ptr<ITaskProcessor>> processors;
        for (size_t i = 0; i < threadCount; ++i) {
            processors.push_back(
                std::unique_ptr<ITaskProcessor>(
                    new ChunkTaskProcessor(filePaths, config, ingestor)));
        }

        if (threadCount > 1)
        {
            m_distributor = Factories::CreateTaskDistributor(processors, filePaths.size());
        }
        else
        {
            // The threadCount == 1 case is implemented to simplify debugging.
            for (size_t i = 0; i < filePaths.size(); ++i) {
                processors[0]->ProcessTask(i);
            }
        }
    }


    void ChunkEnumerator::WaitForCompletion() const
    {
        if (m_distributor != nullptr)
        {
            m_distributor->WaitForCompletion();
        }
    }
}
