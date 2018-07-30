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

#include <iostream>

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Configuration/IStreamConfiguration.h"
#include "BitFunnel/IDiagnosticStream.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IDocumentCache.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/ISimpleIndex.h"
#include "BitFunnel/Plan/Factories.h"
#include "BitFunnel/Plan/IQueryEngine.h"
#include "BitFunnel/Plan/QueryInstrumentation.h"
#include "BitFunnel/Plan/QueryParser.h"     // TODO: Can this move to src/plan?
#include "BitFunnel/Plan/ResultsBuffer.h"
#include "BitFunnel/Plan/VerifyOneQuery.h"
#include "BitFunnel/Utilities/Factories.h"
#include "ByteCodeQueryEngine.h"
#include "MatchVerifier.h"
#include "NativeJITQueryEngine.h"
#include "TermMatchTreeEvaluator.h"


namespace BitFunnel
{
    std::unique_ptr<IMatchVerifier> VerifyOneQuery(
        ISimpleIndex const & index,
        std::string query,
        bool compilerMode)
    {
        // TODO: Get this from ISimpleIndex?
        auto streamConfig = Factories::CreateStreamConfiguration();
        std::unique_ptr<IQueryEngine> queryEngine;
        const size_t c_allocatorSize = 1ull << 17;
        if (compilerMode)
        {
            queryEngine = std::unique_ptr<IQueryEngine>(new NativeJITQueryEngine(index, *streamConfig, c_allocatorSize, c_allocatorSize));
        }
        else
        {
            queryEngine = std::unique_ptr<IQueryEngine>(new ByteCodeQueryEngine(index, *streamConfig, c_allocatorSize));
        }

        auto tree = queryEngine->Parse(query.c_str());

        // TODO: Can MatchVerifier take a char const *? Does it need a copy?
        std::unique_ptr<IMatchVerifier> verifier(new MatchVerifier(query));

        if (tree == nullptr)
        {
            // std::cout << "Empty query." << std::endl;
        }
        else
        {
            auto & cache = index.GetIngestor().GetDocumentCache();
            auto & config = index.GetConfiguration();
            TermMatchTreeEvaluator evaluator(config);

            size_t matchCount = 0;
            size_t documentCount = 0;

            for (auto entry : cache)
            {
                ++documentCount;
                bool matches = evaluator.Evaluate(*tree, entry.first);

                if (matches)
                {
                    ++matchCount;
                    //std::cout
                    //    << "  DocId(" << entry.second << ") "
                    //    << std::endl;

                    verifier->AddExpected(entry.second);
                }
            }

            // std::cout
            //     << matchCount << " match(es) out of "
            //     << documentCount << " documents."
            //     << std::endl;

            auto diagnosticStream = Factories::CreateDiagnosticStream(std::cout);
            // diagnosticStream->Enable("");

            QueryInstrumentation instrumentation;

            ResultsBuffer results(index.GetIngestor().GetDocumentCount());

            queryEngine->Run(tree,
                             instrumentation,
                             results);

            for (auto result : results)
            {
                DocumentHandle handle = Factories::CreateDocumentHandle(result.m_slice, result.m_index);
                verifier->AddObserved(handle.GetDocId());
            }

            verifier->Verify();
            //verifier->Print(std::cout);
        }
        return verifier;
    }
}
