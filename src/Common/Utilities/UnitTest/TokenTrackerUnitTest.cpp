#include <functional>
#include <memory>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

// #include "BitFunnel/Factories.h"
#include "BitFunnel/Utilities/ITaskDistributor.h"
#include "BitFunnel/Utilities/ITaskProcessor.h"
#include "TaskDistributor.h"
#include "TokenTracker.h"

namespace BitFunnel
{
    namespace TokenTrackerUnitTest
    {
        TEST(TokenTrackerBasicTest, Trivial)
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

            std::vector<ITaskProcessor*> m_taskProcessors;
            std::unique_ptr<ITaskDistributor> m_taskDistributor;
            // TokenTracker& m_tracker;
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
        // : m_tracker(tracker)
        {
            for (unsigned i = 0; i < threadCount; ++i)
            {
                m_taskProcessors.push_back(new TokenProcessor(tracker));
            }

            m_taskDistributor.reset(Factories::CreateTaskDistributor(m_taskProcessors, tokenCount));
        }


        TokenDistributor::~TokenDistributor()
        {
            for (unsigned i = 0; i < m_taskProcessors.size(); ++i)
            {
                delete m_taskProcessors[i];
            }
        }


        void TokenDistributor::WaitForCompletion()
        {
            m_taskDistributor->WaitForCompletion();
        }


        TEST(TokenTrackerMultithreadedTest, Trivial)
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
