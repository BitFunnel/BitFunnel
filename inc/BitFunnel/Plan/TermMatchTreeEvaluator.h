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

#include "BitFunnel/TermMatchNode.h"    // Nested classes appear as parameters.


namespace BitFunnel
{
    class IConfiguration;
    class IDocument;

    class TermMatchTreeEvaluator
    {
    public:
        TermMatchTreeEvaluator(IConfiguration const & configuration);

        bool Evaluate(TermMatchNode const & root,
                      IDocument const & document);

    private:
        bool Evaluate(TermMatchNode::And const & tree,
                      IDocument const & document);

        bool Evaluate(TermMatchNode::Not const & tree,
                      IDocument const & document);

        bool Evaluate(TermMatchNode::Or const & tree,
                      IDocument const & document);

        bool Evaluate(TermMatchNode::Unigram const & tree,
                      IDocument const & document);

        IConfiguration const & m_configuration;
    };
}
