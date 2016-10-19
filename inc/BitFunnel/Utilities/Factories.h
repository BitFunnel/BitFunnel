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

#include <iosfwd>
#include <memory>  // For std::unique_ptr.
#include <vector>  // For std::vector.

#include "ITaskDistributor.h"
#include "IThreadManager.h"

namespace BitFunnel
{
    class IBlockAllocator;
    class IDiagnosticStream;
    class IObjectFormatter;
    class ITaskProcessor;
    class ITokenManager;

    namespace Factories
    {
        std::unique_ptr<IBlockAllocator>
            CreateBlockAllocator(size_t blockSize, size_t totalBlockCount);

        std::unique_ptr<IDiagnosticStream> CreateDiagnosticStream(std::ostream& stream);

        // TODO: return unique_ptr.
        IObjectFormatter* CreateObjectFormatter(std::ostream& output);

        std::unique_ptr<ITaskDistributor>
            CreateTaskDistributor(
                std::vector<std::unique_ptr<ITaskProcessor>> const & processors,
                size_t taskCount);

        std::unique_ptr<IThreadManager> CreateThreadManager(
            const std::vector<IThreadBase*>& threads);

        std::unique_ptr<ITokenManager> CreateTokenManager();
    }
}
