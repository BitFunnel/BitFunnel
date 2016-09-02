#pragma once

#include <iosfwd>   // std::istream parameter.


namespace BitFunnel
{
    class IAllocator;
    class TermMatchNode;

    class QueryParser
    {
    public:
        QueryParser(std::istream& input, IAllocator& allocator);

        TermMatchNode const & Parse() const;

    private:
        // OR:
        //   AND (| AND)*
        TermMatchNode const & ParseOr();

        // AND:
        //   TERM ([&] TERM)*
        TermMatchNode const & ParseAnd();

        // TERM:
        //   '-' TERM
        //   '(' OR ')'
        //   '"' PHRASE '"'
        //   UNIGRAM
        TermMatchNode const & ParseTerm();

        // UNIGRAM:
        //   ![SPACE|SPECIAL]*
        TermMatchNode const & ParseUnigram();

        // PHRASE:
        //   '"' UNIGRAM (SPACE* UNIGRAM)* '"'
        TermMatchNode const & ParsePhrase();

        bool AtEOF();
        void SkipWhite();
        char PeekChar();
        char GetChar();

        std::istream& m_input;
        IAllocator& m_allocator;

        bool m_haveChar;
        char m_nextChar;
    };
}
