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


#include "BitFunnel/Index/Factories.h"
#include "Configuration.h"


namespace BitFunnel
{
    std::unique_ptr<IConfiguration>
        Factories::CreateConfiguration(size_t maxGramSize,
                                       bool keepTermText,
                                       IIndexedIdfTable const & idfTable)
    {
        return std::unique_ptr<IConfiguration>(new Configuration(maxGramSize,
                                                                 keepTermText,
                                                                 idfTable));
    }


    Configuration::Configuration(size_t maxGramSize,
                                 bool keepTermText,
                                 IIndexedIdfTable const & idfTable)
        : m_maxGramSize(maxGramSize),
          m_idfTable(idfTable)
    {
        if (keepTermText)
        {
            m_termToText.reset(new TermToText());
        }
    }


    size_t Configuration::GetMaxGramSize() const
    {
        return m_maxGramSize;
    }


    bool Configuration::KeepTermText() const
    {
        return (m_termToText.get() != nullptr);
    }


    TermToText & Configuration::GetTermToText() const
    {
        return *(m_termToText.get());
    }


    IIndexedIdfTable const & Configuration::GetIdfTable() const
    {
        return m_idfTable;
    }
}
