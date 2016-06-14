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


#include "BitFunnel/ITaskProcessor.h"

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


    class ChunkReader : public NonCopyable
    {
    // DESIGN NOTE: Would like to use const char * to avoid string copy and memory
    // allocation during ingestion. This may require reading the entire file into
    // a buffer before parsing.
    // DESIGN NOTE: Need to add arena allocators.
    public:
        // IChunkProcessor? IEventProcessor?
        class IEvents : public IInterface
        {
        public:
            virtual void OnFileEnter() = 0;
            virtual void OnDocumentEnter(DocId id) = 0;
            virtual void OnStreamEnter(char const * name) = 0;
            virtual void OnTerm(char const * term) = 0;
            virtual void OnStreamExit() = 0;
            virtual void OnDocumentExit() = 0;
            virtual void OnFileExit() = 0;
        };

        ChunkReader(std::istream& input, IChunkReaderEvents& processor);
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
