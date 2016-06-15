
#include "BitFunnel/IInterface.h"
#include "BitFunnel/NonCopyable.h"

namespace BitFunnel
{
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
}
