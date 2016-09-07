#pragma once

#include <iosfwd>   // std::istream parameter.
#include <stdexcept>
#include <string>  // std::string.


// Primitive things you can talk about are terms and phrases. And then we'll
// have boolean operators (AND, OR, NOT) and a few other things.
//
// Note that Google and Bing AND terms by default. Lucene uses OR by default.
//
// TODO: what should TERM be called?
//
// This does not support UTF-8. However, as long as we're careful to use the API
// and not do things like call strlen, it should be straightforward to add UTF-8
// support later.
// Consider using http://utfcpp.sourceforge.net/ when we want UTF-8.

namespace BitFunnel
{
    class IAllocator;
    class TermMatchNode;

    class QueryParser
    {
    public:
        QueryParser(std::istream& input, IAllocator& allocator);

        TermMatchNode const * Parse();

        //
        // ParseError records the character position and cause of an error
        // during parsing.
        //
        class ParseError : public std::runtime_error
        {
        public:
            ParseError(char const * message, size_t position);

            friend std::ostream& operator<< (std::ostream &out, const ParseError &e);

        private:
            // Character position where error occurred.
            size_t m_position;
        };

    private:
        // OR:
        //   AND (| AND)*
        TermMatchNode const * ParseOr();

        // AND:
        //   SIMPLE ([&] SIMPLE)*
        TermMatchNode const * ParseAnd();

        // TERM:
        //   [StreamId:]'"' PHRASE '"'
        //   [StreamId:]UNIGRAM
        TermMatchNode const * ParseTerm();

        // SIMPLE:
        //   '-' SIMPLE
        //   '(' OR ')'
        //   TERM
        TermMatchNode const * ParseSimple();

        // UNIGRAM:
        //   ![SPACE|SPECIAL]+
        TermMatchNode const * ParseUnigram();
        TermMatchNode const * ParseCachedUnigram(char const * cache);

        // PHRASE:
        //   '"' UNIGRAM (SPACE* UNIGRAM)* '"'
        TermMatchNode const * ParsePhrase();
        TermMatchNode const * ParseCachedPhrase(char const * cache);

        char const * ParseToken();

        bool AtEOF();
        // Note that delimeters must be ASCII.
        void ExpectDelimeter(char c);
        void SkipWhite();
        char PeekChar();
        char GetChar();
        char GetWithEscape();

        std::istream& m_input;
        IAllocator& m_allocator;

        // Used for errors.
        size_t m_currentPosition;
        bool m_haveChar;
        char m_nextChar;
    };
}
