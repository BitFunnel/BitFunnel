#include <cctype>
#include <sstream>

#include "QueryParser.h"

name BitFunnel
{
    QueryParser(std::istream& input, IAllocator& allocator)
        : m_input(input),
          m_allocator(allocator),
          m_currentPosition(0),
          m_haveChar(false)
    {
    }


    TermMatchNode const & QueryParser::TermMatchNode()
    {
        ParseOr();
    }


    TermMatchNode const & QueryParser::ParseOr()
    {
        TermMatchNode::Builder builder(TermMatchNode::NotMatch, m_allocator);

        auto left = ParseAnd();
        builder.AddChild(left);

        for (;;)
        {
            SkipWhite();
            if (PeekChar() != '|')
            {
                break;
            }
            GetChar();
            auto child = ParseAnd();
            builder.AddChild(child);
        }
        return builder.Complete();
    }


    TermMatchNode const & QueryParser::ParseAnd()
    {
    }


    TermMatchNode const & QueryParser::ParseTerm()
    {
        std::string streamId = "body";

        SkipWhite();
        if (PeekChar() == '"')
        {
            return ParsePhrase(streamId);
        }
        else
        {
            std::string token = PorseToken();

            if (PeekChar() == ':')
            {
                streamId = left;
                GetChar();
            }
            if (PeekChar() == '"')
            {
                return ParsePhrase(streamId);
            }
            else
            {
                return ParseUnigram(streamId);
            }
        }
    }


    TermMatchNode const & QueryParser::ParseSimple()
    {
    }


    TermMatchNode const & QueryParser::ParseUnigram()
    {
    }


    TermMatchNode const & QueryParser::ParsePhrase()
    {
    }


    void QueryParser::SkipWhite()
    {
        while (isspace(PeekChar()))
        {
            GetChar();
        }
    }


    void QueryParser::ExpectDelimeter(char c)
    {
        if (PeekChar() != c)
        {
            // TODO: REVIEW: Check lifetime of c_str() passed to exception constructor.
            std::stringstream message;
            message << "Expected '" << c << "'.";
            throw ParseError(message.str().c_str(), m_currentPosition);
        }
        else
        {
            GetChar();
        }
    }


    char QueryParser::GetChar()
    {
        char result = PeekChar();
        if (result == '\0')
        {
            throw ParseError("Attempting to read past NULL byte",
                             m_currentPosition);
        }
        ++m_currentPosition;
        m_haveChar = false;
        return result;
    }


    char QueryParser::PeekChar()
    {
        if (!m_haveChar)
        {
            if (!m_input.eof())
            {
                m_nextChar = m_input.get();
            }
            else
            {
                m_nextChar = '\0';
            }
            m_haveChar = true;
        }
        return m_nextChar;
    }


    char QueryParser::GetWithEscape()
    {
        char c = PeekChar();
        char const * legalEscapes = "&|\\()\":"
        if (c == '\\')
        {
            GetChar();
            c = PeekChar();
            if (strchr(legalEscapes, c) != nullptr)
            {
                return GetChar();
            }
            else
            {
                throw ParseError("Bad escape char",
                                 c);
            }
        }
        else
        {
            return GetChar();
        }
    }


    Parser::ParseError::ParseError(char const * message, size_t position)
        : std::runtime_error(message),
          m_position(position)
    {
    }


    std::ostream& operator<< (std::ostream &out, const Parser::ParseError &e)
    {
        out << std::string(e.m_position, ' ') << '^' << std::endl;
        out << "Parser error (position = " << e.m_position << "): ";
        out << e.what();
        out << std::endl;
        return out;
    }
}
