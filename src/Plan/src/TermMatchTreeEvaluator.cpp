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

#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Index/IDocument.h"
#include "BitFunnel/Plan/TermMatchTreeEvaluator.h"
#include "BitFunnel/Term.h"


namespace BitFunnel
{
    TermMatchTreeEvaluator::TermMatchTreeEvaluator(
        IConfiguration const & configuration)
        : m_configuration(configuration)
    {
    }


    bool TermMatchTreeEvaluator::Evaluate(
        TermMatchNode const & node,
        IDocument const & document)
    {
        switch (node.GetType())
        {
        case TermMatchNode::AndMatch:
            return Evaluate(dynamic_cast<const TermMatchNode::And&>(node), document);
        case TermMatchNode::NotMatch:
            return Evaluate(dynamic_cast<const TermMatchNode::Not&>(node), document);
        case TermMatchNode::OrMatch:
            return Evaluate(dynamic_cast<const TermMatchNode::Or&>(node), document);
        //case TermMatchNode::PhraseMatch:
        //    return Evaluate(dynamic_cast<const TermMatchNode::Phrase&>(node), document);
        case TermMatchNode::UnigramMatch:
            return Evaluate(dynamic_cast<const TermMatchNode::Unigram&>(node), document);
        default:
            RecoverableError error("TermMatchTreeEvaluator::Evaluate:: Invalid node type.");
            throw error;
        }
    }

    bool TermMatchTreeEvaluator::Evaluate(
        TermMatchNode::And const & tree,
        IDocument const & document)
    {
        return Evaluate(tree.GetLeft(), document)
            && Evaluate(tree.GetRight(), document);
    }


    bool TermMatchTreeEvaluator::Evaluate(
        TermMatchNode::Not const & tree,
        IDocument const & document)
    {
        return !Evaluate(tree.GetChild(), document);
    }


    bool TermMatchTreeEvaluator::Evaluate(
        TermMatchNode::Or const & tree,
        IDocument const & document)
    {
        return Evaluate(tree.GetLeft(), document)
            || Evaluate(tree.GetRight(), document);
    }


    bool TermMatchTreeEvaluator::Evaluate(
        TermMatchNode::Unigram const & tree,
        IDocument const & document)
    {
        Term term(tree.GetText(), tree.GetStreamId(), m_configuration);
        return document.Contains(term);
    }
}
