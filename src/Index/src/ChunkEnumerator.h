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

#include <memory>       // std::unique_ptr member.
#include <stddef.h>     // size_t parameter.
#include <string>       // std::string template parameter.

#include "BitFunnel/NonCopyable.h"                  // Inherits from NonCopyable.
#include "BitFunnel/Utilities/ITaskDistributor.h"   // std::unqiue_ptr template parameter.


namespace BitFunnel
{
    class IConfiguration;
    class IIngestor;

    // Fill an std::vector with filenames
    // Construct a ChunkTaskProcessor for each thread
    // Pass the above to constructor TaskDistributor()
    class ChunkEnumerator : public NonCopyable
    {
    public:
        ChunkEnumerator(std::vector<std::string> const & filePaths,
                        IConfiguration const & config,
                        IIngestor& ingestor,
                        size_t threadCount);

        void WaitForCompletion() const;

    private:
        std::unique_ptr<ITaskDistributor> m_distributor;
    };
}
