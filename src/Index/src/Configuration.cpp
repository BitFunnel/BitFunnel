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


#include <memory>

#include "BitFunnel/Index/Factories.h"
#include "Configuration.h"


namespace BitFunnel
{
    std::unique_ptr<IConfiguration>
        Factories::CreateConfiguration(size_t maxGramSize)
    {
        return std::unique_ptr<IConfiguration>(new Configuration(maxGramSize));
    }


    Configuration::Configuration(size_t maxGramSize)
        : m_maxGramSize(maxGramSize)
    {
    }


    size_t Configuration::GetMaxGramSize() const
    {
        return m_maxGramSize;
    }


    IDocumentFrequencyTable const &
        Configuration::GetDocumentFrequencyTable() const
    {
        // TODO: Implement this method.
        return *static_cast<IDocumentFrequencyTable *>(nullptr);
    }
}
