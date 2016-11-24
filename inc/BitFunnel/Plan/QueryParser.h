// The MIT License (MIT)

// Copyright (c) 2016, Microsoft

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once

#include <iosfwd>                   // std::istream parameter.
#include <string>                   // std::string return value.

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
        QueryParser(char const * input,
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

        // Escapes a string that contains characters that would otherwise have
        // special meaning to QueryParser.
        static std::string Escape(char const * input);

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

        char const * m_input;
        IStreamConfiguration const & m_streamConfiguration;
        IAllocator& m_allocator;

        // Used for errors.
        size_t m_currentPosition;
        bool m_haveChar;
        char m_nextChar;

        static char const * m_escapeChars;
    };
}
