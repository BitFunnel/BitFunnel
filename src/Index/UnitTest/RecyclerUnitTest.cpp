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

// TODO: port this test.

#include "BasicThreadPool.h"
#include "BitFunnel/Factories.h"
#include "BitFunnel/Stopwatch.h"
#include "BitFunnel/Token.h"
#include "LockGuard.h"
#include "Recycler.h"
#include "SuiteCpp/UnitTest.h"
#include "ThreadAction.h"

namespace BitFunnel
{
    namespace RecyclerUnitTest
    {
        // Verify basic starting and stopping of the recycler.
        TestCase(StartStopTest)
        {
            Recycler /* recycler */;

            // Give worker thread a chance to start the loop.
            Sleep(10);
        }


        // Class which represents an IRecyclable whose lifetime is controlled by a Token
        // obtained from the ITokenManager.
        class TokenBasedRecyclable : public IRecyclable
        {
        public:
            TokenBasedRecyclable(ITokenManager& tokenManager, volatile bool& hasRecycled);

            ~TokenBasedRecyclable();

            bool HasRecycled() const;

            //
            // IRecyclable API.
            //
            virtual bool TryRecycle() override;

        private:
            mutable Mutex m_lock;
            volatile bool& m_hasRecycledFlag;
            const std::shared_ptr<ITokenTracker> m_tracker;
        };


        TokenBasedRecyclable::TokenBasedRecyclable(ITokenManager& tokenManager, volatile bool& hasRecycled)
            : m_hasRecycledFlag(hasRecycled),
              m_tracker(tokenManager.StartTracker())
        {
        }


        TokenBasedRecyclable::~TokenBasedRecyclable()
        {
            TestAssert(m_hasRecycledFlag);
        }


        bool TokenBasedRecyclable::HasRecycled() const
        {
            LockGuard lock(m_lock);

            return m_hasRecycledFlag;
        }


        bool TokenBasedRecyclable::TryRecycle()
        {
            LockGuard lock(m_lock);
            if (m_tracker->IsComplete())
            {
                m_hasRecycledFlag = true;

                return true;
            }
            else
            {
                return false;
            }
        }


        TestCase(RecyclingTest)
        {
            BasicThreadPool threadPool;
            Recycler recycler(threadPool.Get());

            std::unique_ptr<ITokenManager> tokenManager(Factories::CreateTokenManager());

            // Give worker thread a chance to start the loop.
            Sleep(10);

            {
                volatile bool hasRecycled = false;
                std::unique_ptr<IRecyclable> recyclable(
                    new TokenBasedRecyclable(*tokenManager, hasRecycled));

                recycler.ScheduleRecyling(recyclable);

                // No tokens held - should be recycled fast.
                Sleep(3);
                TestAssert(hasRecycled);
            }

            {
                // Get 2 tokens. Hold one for 100 ms and the other for 200ms.
                // Recycler should finish within a reasonable proximity of 200ms.
                ThreadAction token1Requester([&]() {
                    const Token token = tokenManager->RequestToken();
                    Sleep(100);
                });

                ThreadAction token2Requester([&]() {
                    const Token token = tokenManager->RequestToken();
                    Sleep(200);
                });

                // Measure the latency of recycling.
                Stopwatch timer;
                unsigned recycleLatencyMS = 0;

                // Give requester threads a chance to start.
                Sleep(10);

                volatile bool hasRecycled = false;
                std::unique_ptr<IRecyclable> recyclable(
                    new TokenBasedRecyclable(*tokenManager, hasRecycled));

                recycler.ScheduleRecyling(recyclable);

                ThreadAction verifier([&]() {
                    for (;;)
                    {
                        if (hasRecycled)
                        {
                            recycleLatencyMS = static_cast<unsigned>(timer.ElapsedTime() * 1000);
                            break;
                        }

                        Sleep(1);
                    }
                });

                TestAssert(token1Requester.WaitForCompletion(1000));
                TestAssert(token2Requester.WaitForCompletion(1000));
                TestAssert(verifier.WaitForCompletion(1000));

                TestAssert(recycleLatencyMS > 200 && recycleLatencyMS < 300);
            }

            tokenManager->Shutdown();
        }
    }
}
