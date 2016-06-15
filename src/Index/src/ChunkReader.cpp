

#include "ChunkEnumerator.h" // TODO: change to appropriate file.

// TODO: change to avoid creating std::string objects while running.

namespace BitFunnel
{
    ChunkReader::ChunkReader(std::istream& input, IChunkReaderEvents& processor)
    {
        processor.OnFileEnter();
        while (input.peek() != '\0')
        {
            ProcessDocument(input, processor);
        }
        ProcessNull(input);
        processor.OnFileExit();
    }


    void ChunkReader::ProcessDocument(std::istream& input, IChunkReaderEvents& processor)
    {
        // TODO: get a doc id.
        processor.OnDocumentEnter(-1);
        while (input.peek() != '\0')
        {
            ProcessStream(input, processor);
        }
        ProcessNull(input);
        processor.OnDocumentExit();
    }


    void ChunkReader::ProcessStream(std::istream& input, IChunkReaderEvents& processor)
    {
        std::string token;
        std::getline(input, token, '\0');
        processor.OnStreamEnter(token.c_str());
        while (input.peek() != '\0')
        {
            std::getline(input, token, '\0');
            processor.OnTerm1(token.c_str());
        }
        ProcessNull(input);
        processor.OnStreamExit();
    }


    void ChunkReader::ProcessNull(std::istream& input)
    {
        char maybeNull;
        input.get(maybeNull);
        if (maybeNull != '\0')
        {
            throw std::runtime_error("Expected \0, got " + maybeNull);
        }
    }
}
