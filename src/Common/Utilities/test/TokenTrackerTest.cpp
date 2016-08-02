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
#include <vector>

#include "gtest/gtest.h"

#include "BitFunnel/Utilities/ITaskDistributor.h"
#include "BitFunnel/Utilities/ITaskProcessor.h"
#include "BitFunnel/Utilities/Factories.h"
#include "TokenTracker.h"

namespace BitFunnel
{
    namespace TokenTrackerTest
    {
        TEST(TokenTracker, Basic)
        {
            static const SerialNumber c_anyCutoffSerialNumber = 10;
            static_assert(c_anyCutoffSerialNumber > 1,
            "For test purposes c_anyCutoffSerialNumber must be greater than 1");

            static const unsigned c_anyInFlightTokenCount = 5;
            static_assert(c_anyInFlightTokenCount > 1,
            "For test purposes c_anyInFlightTokenCount must be greater than 1");

            TokenTracker tracker(c_anyCutoffSerialNumber,
                                 c_anyInFlightTokenCount);

            ASSERT_TRUE(!tracker.IsComplete());

            // Returning c_anyInFlightTokenCount - 1 tokens.
            for (unsigned i = 0; i < c_anyInFlightTokenCount - 1; ++i)
            {
                // This should decrement the number of in flight tokens.
                tracker.OnTokenComplete(c_anyCutoffSerialNumber - 1);
            }

            // One token is still in flight.
            ASSERT_TRUE(!tracker.IsComplete());

            // This is a token outside of our tracking interest, it will not
            // affect tracking.
            tracker.OnTokenComplete(c_anyCutoffSerialNumber + 1);
            ASSERT_TRUE(!tracker.IsComplete());

            // Returning the last token, tracking should be complete.
            tracker.OnTokenComplete(c_anyCutoffSerialNumber - 1);
            ASSERT_TRUE(tracker.IsComplete());
        }


        // A wrapper class which is responsible of issuing multiple calls to
        // the passed in's ITracker::OnTokenComplete.
        class TokenDistributor : private NonCopyable
        {
        public:
            // Creates tokenCount tasks for treadCount task processors and
            // waits for all tasks to finish.
            TokenDistributor(TokenTracker& tracker,
                             unsigned threadCount,
                             unsigned tokenCount);

            ~TokenDistributor();

            void WaitForCompletion();

        private:

            class TokenProcessor : public ITaskProcessor,
                                   private NonCopyable
            {
            public:

                TokenProcessor(TokenTracker& tracker);

                //
                // ITaskProcessor API
                //
                virtual void ProcessTask(size_t taskId) override;
                virtual void Finished() override;

            private:
                TokenTracker& m_tracker;

            };

            std::vector<std::unique_ptr<ITaskProcessor>> m_taskProcessors;
            std::unique_ptr<ITaskDistributor> m_taskDistributor;
        };


        //*********************************************************************
        //
        // TokenProcessor.
        //
        //*********************************************************************
        TokenDistributor::TokenProcessor::TokenProcessor(TokenTracker& tracker)
            : m_tracker(tracker)
        {
        }


        void TokenDistributor::TokenProcessor::ProcessTask(size_t taskId)
        {
            const SerialNumber serialNumber = static_cast<SerialNumber>(taskId);
            m_tracker.OnTokenComplete(serialNumber);
        }


        void TokenDistributor::TokenProcessor::Finished()
        {
            // No-op.
        }


        //*********************************************************************
        //
        // TokenDistributor.
        //
        //*********************************************************************
        TokenDistributor::TokenDistributor(TokenTracker& tracker,
                                           unsigned threadCount,
                                           unsigned tokenCount)
        {
            for (unsigned i = 0; i < threadCount; ++i)
            {
                m_taskProcessors.push_back(
                    std::unique_ptr<ITaskProcessor>(new TokenProcessor(tracker)));
            }

            m_taskDistributor =
                Factories::CreateTaskDistributor(m_taskProcessors, tokenCount);
        }


        TokenDistributor::~TokenDistributor()
        {
        }


        void TokenDistributor::WaitForCompletion()
        {
            m_taskDistributor->WaitForCompletion();
        }


        TEST(TokenTracker, MultithreadedTest)
        {
            // We will create c_anyTotalTokenCount with serial numbers monotonically
            // increasing starting from 0. Out of all of them, we are interested in
            // tracking tokens with serial number cutoff of c_anyCutoffSerialNumber.
            // There will be c_anyCutoffSerialNumber such tokens.
            static const unsigned c_anyTotalTokenCount = 50;
            static const SerialNumber c_anyCutoffSerialNumber = 10;

            static const unsigned c_anyThreadCount = 20;

            // Since serial numbers are monotonically increasing starting from 1,
            // the number of tokens in flight is the same as the cutoff serial
            // number.
            TokenTracker tracker(c_anyCutoffSerialNumber,  c_anyCutoffSerialNumber);
            ASSERT_TRUE(!tracker.IsComplete());

            TokenDistributor distributor(tracker, c_anyThreadCount, c_anyTotalTokenCount);
            distributor.WaitForCompletion();

            ASSERT_TRUE(tracker.IsComplete());
        }

        // TODO: create a test that actually calls .IsComplete, etc., from
        // other threads
   }
}
