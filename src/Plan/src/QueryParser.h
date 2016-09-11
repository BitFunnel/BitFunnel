#pragma once

#include <iosfwd>                   // std::istream parameter.

#include "BitFunnel/Exceptions.h"   // Base class.
#include "BitFunnel/Term.h"         // Term::StreamId parameter.


// Primitive things you can talk about are terms and phrases. And then we'll
// have boolean operators (AND, OR, NOT) and a few other things.
//
// Note that Google and Bing AND terms by default. Lucene uses OR by default.
//
// TODO: what should TERM be called?
//
// This doesn't have any unicode specific support. However, since UTF-8 bytes
// cannot be ASCII characters unless the UTF-8 character actually is the ASCII
// character, the operator and escaping code here should work with UTF-8.

namespace BitFunnel
{
    class IAllocator;
    class IStreamConfiguration;
    class TermMatchNode;

    class QueryParser
    {
    public:
        QueryParser(std::istream& input,
                    IStreamConfiguration const & streamConfiguration,
                    IAllocator& allocator);

        TermMatchNode const * Parse();

        //
        // ParseError records the character position and cause of an error
        // during parsing.
        //
        class ParseError : public RecoverableError
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
        //   SIMPLE (['&'] SIMPLE)*
        TermMatchNode const * ParseAnd();

        // SIMPLE:
        //   '-' SIMPLE
        //   '(' OR ')'
        //   TERM
        TermMatchNode const * ParseSimple();

        // TERM:
        //   [StreamId:]'"' PHRASE '"'
        //   [StreamId:]UNIGRAM
        TermMatchNode const * ParseTerm();

        // PHRASE:
        //   '"' UNIGRAM (SPACE* UNIGRAM)* '"'
        TermMatchNode const * ParsePhrase(Term::StreamId streamId);

        // UNIGRAM:
        //   [![SPACE|SPECIAL] | ESCAPE]+
        char const * ParseToken();

        // DESIGN NOTE:
        // one-two is parsed as one NOT two instead of the unigram one-two.
        // This should probably be changed as this is counter-intuitive for
        // most users.


        // Note that delimeters must be ASCII.
        void ExpectDelimeter(char c);
        void SkipWhite();

        char GetWithEscape();
        char GetChar();
        char PeekChar();

        Term::StreamId StreamIdFromText(char const * /*streamName*/) const;

        std::istream& m_input;
        IStreamConfiguration const & m_streamConfiguration;
        IAllocator& m_allocator;

        // Used for errors.
        size_t m_currentPosition;
        bool m_haveChar;
        char m_nextChar;
    };
}
