#include <iostream> // TODO: remove.

#include <cctype>
#include <istream>
#include <sstream>

#include "BitFunnel/Allocators/IAllocator.h"
#include "BitFunnel/TermMatchNode.h"
#include "QueryParser.h"

namespace BitFunnel
{
    QueryParser::QueryParser(std::istream& input, IAllocator& allocator)
        : m_input(input),
          m_allocator(allocator),
          m_currentPosition(0),
          m_haveChar(false)
    {
    }


    TermMatchNode const * QueryParser::Parse()
    {
        return ParseOr();
        // TODO: parse the correct thing.
        // return ParseOr();
    }


    TermMatchNode const * QueryParser::ParseOr()
    {
        TermMatchNode::Builder builder(TermMatchNode::OrMatch, m_allocator);

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


    TermMatchNode const * QueryParser::ParseAnd()
    {
        // TODO: parse a sequence of SIMPLEs.
        return ParseSimple();
    }


    TermMatchNode const * QueryParser::ParseTerm()
    {
        Term::StreamId streamId = 0; // TODO: convert streamId to string?

        SkipWhite();
        if (PeekChar() == '"')
        {
            // TODO: handle streamId.
            // return ParsePhrase(streamId);
            return ParsePhrase();
        }
        else
        {
            const char * unigram = ParseToken();

            if (PeekChar() == ':')
            {
                // streamId = token;
                streamId = 0;
                GetChar();
            }
            else
            {
                return ParseCachedUnigram(unigram);
            }

            // TODO: refactor this into multiple productions to simplify.
            // If we're here, we saw a ":"

            if (PeekChar() == '"')
            {
                // TODO: add streamId.
                // return ParsePhrase(streamId);
                return ParsePhrase();
            }
            else
            {
                // TODO: add streamId.
                // return ParseUnigram(streamId);
                return ParseUnigram();
            }
        }
    }


    TermMatchNode const * QueryParser::ParseSimple()
    {
        // TODO: handle - / NOT.
        if (PeekChar() == '(')
        {
            GetChar();
            auto orNode = ParseOr();
            SkipWhite();
            ExpectDelimeter(')');
            return orNode;
        }
        else
        {
            return ParseTerm();
        }
    }


    TermMatchNode const * QueryParser::ParseUnigram()
    {
        char const * token = ParseToken();

        Term::StreamId dummy = 0;
        return TermMatchNode::Builder::CreateUnigramNode(token, dummy, m_allocator);
    }


    TermMatchNode const * QueryParser::ParseCachedUnigram(char const * cache)
    {
        Term::StreamId dummy = 0;
        return TermMatchNode::Builder::CreateUnigramNode(cache, dummy, m_allocator);
    }



    TermMatchNode const * QueryParser::ParsePhrase()
    {
        TermMatchNode::Builder builder(TermMatchNode::OrMatch, m_allocator);
        // TODO.
        return builder.Complete();
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
            message << "Expected '" << c << "' Got '" << PeekChar() << "'";
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
            int temp = m_input.get();
            // See https://github.com/BitFunnel/BitFunnel/issues/189.
            if (temp != -1)
            {
                // TODO: when we handle UTF-8 correctly, everything will turn
                // into int.
                m_nextChar = static_cast<char>(temp);
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
        char const * legalEscapes = "&|\\()\":";
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


    char const * QueryParser::ParseToken()
    {
        // TODO: unify with legalEscapes.
        char const * specialChars = "&|\\()\":";

        std::string token;
        char c = PeekChar();
        if (isspace(c) || strchr(specialChars, c) != nullptr)
        {
            // TODO: should we throw here or just return the empty string?
            throw ParseError("Found space or special character at beginning of unigram.",
                             m_currentPosition);

        }
        do
        {
            char temp = GetChar();
            token.push_back(temp);
            c = PeekChar();
        } while (!isspace(c) && strchr(specialChars, c) == nullptr);

        char* buffer = static_cast<char*>(m_allocator.Allocate(token.size()+1));
        memcpy(buffer, token.c_str(), token.size()+1);
        std::cout << "ParseToken result: " << token << ":" << buffer << "(" << token.size() << ")" <<std::endl;
        return buffer;
    }


    QueryParser::ParseError::ParseError(char const * message, size_t position)
        : std::runtime_error(message),
          m_position(position)
    {
    }


    std::ostream& operator<< (std::ostream &out, const QueryParser::ParseError &e)
    {
        out << std::string(e.m_position, ' ') << '^' << std::endl;
        out << "Parser error (position = " << e.m_position << "): ";
        out << e.what();
        out << std::endl;
        return out;
    }
}
