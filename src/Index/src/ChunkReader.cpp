#include <sstream>

#include "BitFunnel/Exceptions.h"
#include "ChunkReader.h"


namespace BitFunnel
{
    ChunkReader::ChunkReader(std::vector<char> const & input, IEvents& processor)
      : m_input(input),
        m_processor(processor),
        m_next(&input[0]),
        m_end(&input[0] + input.size())
    {
        m_processor.OnFileEnter();
        while (PeekChar() != 0) {
            ProcessDocument();
        }

        Consume(0);
        m_processor.OnFileExit();
    }


    void ChunkReader::ProcessDocument()
    {
        // TODO: Get doc id.
        m_processor.OnDocumentEnter(0);
        while (PeekChar() != 0) {
            ProcessStream();
        }

        Consume(0);

        m_processor.OnDocumentExit();
    }


    void ChunkReader::ProcessStream()
    {
        m_processor.OnStreamEnter(GetToken());
        while (PeekChar() != 0) {
            m_processor.OnTerm(GetToken());
        }

        Consume(0);

        m_processor.OnStreamExit();
    }


    char const * ChunkReader::GetToken()
    {
        char const * begin = m_next;

        while (PeekChar() != 0) {
            GetChar();
        }

        Consume(0);

        return begin;
    }


    char ChunkReader::PeekChar()
    {
        if (m_next == m_end) {
            throw FatalError("Attempt to read beyond end of buffer.");
        }
        else
        {
            return *m_next;
        }
    }


    char ChunkReader::GetChar()
    {
        if (m_next == m_end) {
            throw FatalError("Attempt to read beyond end of buffer.");
        }
        else
        {
            return *m_next++;
        }
    }

    void ChunkReader::Consume(char c)
    {
        if (PeekChar() != c) {
            std::stringstream msg;
            msg << "Expected character " << c << ". ";
            msg << "Found character " << PeekChar() << ".";
            throw FatalError(msg.str());
        }

        GetChar();
    }
}
