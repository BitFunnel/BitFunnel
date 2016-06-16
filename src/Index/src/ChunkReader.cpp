
#include <istream>
#include <sstream>

#include "ChunkEnumerator.h" // TODO: change to appropriate file.

// TODO: change to avoid creating std::string objects while running.

namespace BitFunnel
{
    ChunkReader::ChunkReader(std::vector<char> const & input, IEvents& processor)
      : m_haveChar(false),
        m_index(0),
        m_input(input),
        m_processor(processor)
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
        m_processor.OnStreamEnter(GetToken().c_str());
        while (PeekChar() != 0) {
            m_processor.OnTerm(GetToken().c_str());
        }

        Consume(0);

        m_processor.OnStreamExit();
    }

    char ChunkReader::PeekChar()
    {
        if (!m_haveChar) {
            if (m_index >= m_input.size()) {
                throw 0;
            }

            m_current = m_input[m_index++];
            m_haveChar = true;
        }

        return m_current;
    }

    std::string ChunkReader::GetToken()
    {
        std::string s;
        while (PeekChar() != 0) {
            s.push_back(GetChar());
        }

        Consume(0);

        return s;
    }

    char ChunkReader::GetChar()
    {
        PeekChar();

        if (!m_haveChar) {
            throw 0;
        }

        m_haveChar = false;

        return m_current;
    }

    void ChunkReader::Consume(char c)
    {
        if (PeekChar() != c) {
            throw 0;
        }

        GetChar();
    }
}
