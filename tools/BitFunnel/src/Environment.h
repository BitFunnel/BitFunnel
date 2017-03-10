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

#include <memory>                           // std::unique_ptr embedded.

#include "BitFunnel/Index/ISimpleIndex.h"   // Parameterizes std::unique_ptr.
#include "BitFunnel/NonCopyable.h"          // Base class.
#include "BitFunnel/Term.h"                 // Term::GramSize embedded.
#include "TaskFactory.h"                    // Parameterizes std::unique_ptr.
#include "TaskPool.h"                       // Parameterizes std::unique_ptr.


namespace BitFunnel
{
    class IFileSystem;
    class TaskFactory;
    class TaskPool;

    class Environment : public NonCopyable
    {
    public:
        Environment(IFileSystem& fileSystem,
                    std::ostream& output,
                    char const * directory,
                    size_t gramSize,
                    size_t threadCount);

        void StartIndex();

        IFileSystem & GetFileSystem() const;

        bool GetCacheLineCountMode() const;
        void SetCacheLineCountMode(bool mode);

        bool GetCompilerMode() const;
        void SetCompilerMode(bool mode);

        bool GetFailOnException() const;
        void SetFailOnException(bool mode);

        std::string const & GetOutputDir() const;
        void SetOutputDir(std::string dir);

        std::ostream & GetOutputStream() const;

        size_t GetThreadCount() const;
        void SetThreadCount(size_t threadCount);

        TaskFactory & GetTaskFactory() const;
        TaskPool & GetTaskPool() const;
        IConfiguration const & GetConfiguration() const;
        ISimpleIndex const & GetSimpleIndex() const;
        IIngestor & GetIngestor() const;
        ITermTable const & GetTermTable() const;

    private:
        void RegisterCommands();

        IFileSystem& m_fileSystem;

        std::unique_ptr<TaskFactory> m_taskFactory;
        std::unique_ptr<TaskPool> m_taskPool;
        std::unique_ptr<ISimpleIndex> m_index;

        bool m_cacheLineCountMode;
        bool m_compilerMode;
        bool m_failOnException;
        size_t m_threadCount;
        std::string m_outputDir;
        std::ostream& m_output;
    };
}
