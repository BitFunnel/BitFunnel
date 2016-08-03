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


#include <functional>
#include <future>
#include <memory>
#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Index/IConfiguration.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Row.h"
#include "BitFunnel/RowId.h"
#include "ChunkTaskProcessor.h"
#include "DocumentDataSchema.h"
#include "EmptyTermTable.h"
#include "IRecycler.h"
#include "MockFileManager.h"
#include "Recycler.h"
#include "SliceBufferAllocator.h"


namespace BitFunnel
{
    namespace ChunkTaskProcessorTest
    {
        // Sets up and configures a `ChunkTaskProcessor` (according to
        // `filePaths` and `ngramSize`), and passes that task processor to the
        // `test` function, which is intended to contain all of the test logic
        // (i.e., the `ASSERT`s and `EXPECT`s that verify behavior of the task
        // processor.) See tests below for examples.
        static void RunTest(
            std::vector<std::string> const & filePaths,
            size_t ngramSize,
            std::function<void(ChunkTaskProcessor &)> const & test)
        {
            auto fileManager = CreateMockFileManager();

            const std::unique_ptr<IConfiguration>
                configuration(Factories::CreateConfiguration(ngramSize));


            std::unique_ptr<IRecycler> recycler =
                std::unique_ptr<IRecycler>(new Recycler());
            auto background = std::async(std::launch::async, &IRecycler::Run, recycler.get());

            // Create dummy SliceBufferAllocator to satisfy interface.
            // TODO: fix constants.
            std::unique_ptr<ISliceBufferAllocator> sliceBufferAllocator =
                std::unique_ptr<ISliceBufferAllocator>(
                    new SliceBufferAllocator(2048*24, 1));

            static const std::vector<RowIndex>
                rowCounts = { c_systemRowCount, 0, 0, 1, 0, 0, 1 };
            std::shared_ptr<ITermTable const>
                termTable(new EmptyTermTable(rowCounts));

            DocumentDataSchema schema;

            const std::unique_ptr<IIngestor>
                ingestor(Factories::CreateIngestor(*fileManager,
                                                   schema,
                                                   *recycler,
                                                   *termTable,
                                                   *sliceBufferAllocator));

            ChunkTaskProcessor processor(filePaths, *configuration, *ingestor);

            test(processor);

            ingestor->Shutdown();
            recycler->Shutdown();
        }


        TEST(ChunkTaskProcessor, Basic)
        {
            // TODO: What's the Bing style for lambda brace placement?
            RunTest({}, 1, [](ChunkTaskProcessor & processor)
            {
                // Throw when `taskId` is the size of the empty array (i.e.,
                // protect against boundary condition errors).
                EXPECT_ANY_THROW(processor.ProcessTask(0));
            });
        }


        TEST(ChunkTaskProcessor, BadTaskIds)
        {
            const std::vector<std::string> filePaths { "fileThatDoesNotExist" };
            RunTest(filePaths, 3, [](ChunkTaskProcessor & processor) {
                // Throw when we try to read a file with an invalid name.
                EXPECT_ANY_THROW(processor.ProcessTask(0));

                // Throw for generic out-of-bounds errors.
                EXPECT_ANY_THROW(processor.ProcessTask(3));

                // Throw for negative numbers. On most platforms, this will end
                // up being a really large number. NOTE: Clang and GCC don't
                // warn about this case, but MSVC will error out because of the
                // signed/unsigned conversion.
#ifndef _MSC_VER
                EXPECT_ANY_THROW(processor.ProcessTask(-1));
#endif

                // Throw when `taskId` is the size of the array (i.e., protect
                // against boundary condition errors).
                EXPECT_ANY_THROW(processor.ProcessTask(1));
            });
        }
    }
}
