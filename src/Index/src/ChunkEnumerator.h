#pragma once

#include <string>
#include "BitFunnel/NonCopyable.h"

namespace BitFunnel
{
    class IIndex;

    // Fill an std::vector with filenames
    // Construct a ChunkTaskProcessor for each thread
    // Pass the above to constructor TaskDistributor()
    class ChunkEnumerator : public NonCopyable
    {
    public:
        ChunkEnumerator(std::string const & path, IIndex& index, size_t threadCount);
        void WaitForCompletion() const;
    
    private:
        IIndex& m_index;
    };
}


#include "BitFunnel/Utilities/ITaskProcessor.h"

namespace BitFunnel
{
    class ChunkTaskProcessor :public ITaskProcessor
    {
    public:
        ChunkTaskProcessor(std::vector<std::string const> const & filePaths, IIndex& index);

        //
        // ITaskProcessor methods
        //
        virtual void ProcessTask(size_t taskId) override;
        virtual void Finished() override;

    private:
        std::vector<std::string const> const & m_filePaths;
        IIndex& m_index;

    };


    // DESIGN NOTE: Consider adding a document factory parameter to the constructor.
    class ChunkIngestor : public NonCopyable, public ChunkReader::IEvents
    {
    public:
        ChunkIngestor(std::string const & filePath, IIndex& index, IDocumentFactory& factory);

    private:
        std::string const & m_filePath;
        IIndex& m_index;

    };
}
