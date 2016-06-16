#include <functional>
#include <memory>

#include "BitFunnel/Factories.h"
#include "BitFunnel/ITaskDistributor.h"
#include "BitFunnel/ITaskProcessor.h"
#include "BitFunnel/Stopwatch.h"
#include "SuiteCpp/UnitTest.h"
#include "TokenTracker.h"

namespace BitFunnel
{
    namespace TokenTrackerUnitTest
    {
        TestCase(TokenTrackerBasicTest)
        {
            static const SerialNumber c_anyCutoffSerialNumber = 10;
            static_assert(c_anyCutoffSerialNumber > 1, 
                          "For test purposes c_anyCutoffSerialNumber must be greater than 1");

            static const unsigned c_anyInFlightTokenCount = 5;
            static_assert(c_anyInFlightTokenCount > 1, 
                          "For test purposes c_anyInFlightTokenCount must be greater than 1");

            TokenTracker tracker(c_anyCutoffSerialNumber, 
                                 c_anyInFlightTokenCount);

            TestAssert(!tracker.IsComplete());

            // Returning c_anyInFlightTokenCount - 1 tokens.
            for (unsigned i = 0; i < c_anyInFlightTokenCount - 1; ++i)
            {
                // This should decrement the number of in flight tokens.
                tracker.OnTokenComplete(c_anyCutoffSerialNumber - 1);
            }

            // One token is still in flight.
            TestAssert(!tracker.IsComplete());

            // This is a token outside of our tracking interest, it will not
            // affect tracking.
            tracker.OnTokenComplete(c_anyCutoffSerialNumber + 1);
            TestAssert(!tracker.IsComplete());

            // Returning the last token, tracking should be complete.
            tracker.OnTokenComplete(c_anyCutoffSerialNumber - 1);
            TestAssert(tracker.IsComplete());
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

            bool WaitForCompletion(int timeoutInMs);

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
            TokenTracker& m_tracker;
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
            : m_tracker(tracker)
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


        bool TokenDistributor::WaitForCompletion(int timeoutInMs)
        {
            return m_taskDistributor->WaitForCompletion(timeoutInMs);
        }


        TestCase(TokenTrackerMultithreadedTest)
        {
            // We will create c_anyTotalTokenCount with serial numbers monotonically
            // increasing starting from 0. Out of all of them, we are interested in
            // tracking tokens with serial number cutoff of c_anyCutoffSerialNumber.
            // There will be c_anyCutoffSerialNumber such tokens.
            static const unsigned c_anyTotalTokenCount = 50;
            static const SerialNumber c_anyCutoffSerialNumber = 10;

            static const unsigned c_tokenDistributorTimeoutInMS = 60 * 1000;
            static const unsigned c_anyThreadCount = 20;

            // Since serial numbers are monotonically increasing starting from 1,
            // the number of tokens in flight is the same as the cutoff serial 
            // number.
            TokenTracker tracker(c_anyCutoffSerialNumber,  c_anyCutoffSerialNumber);
            TestAssert(!tracker.IsComplete());

            TokenDistributor distributor(tracker, c_anyThreadCount, c_anyTotalTokenCount);
            distributor.WaitForCompletion(c_tokenDistributorTimeoutInMS);

            TestAssert(tracker.IsComplete());
        }


        // TODO : this class is used in multiple tests, consider moving to a 
        // shared location.
        //
        // Class which represents an action which needs to be performed on a
        // new thread. Takes the action via an argument to the constructor,
        // launches a new thread, executes an action and destroyes a thread.
        class ThreadAction : private NonCopyable
        {
        public:

            // Starts a thread, executes an action and stops a thread.
            ThreadAction(const std::function<void()> action);

            // Closes thread's handle.
            ~ThreadAction();

            // Performs an action which was assigned to a thread.
            void Action();

            // Waits for the action to complete for a given timeout.
            bool WaitForCompletion(unsigned timeoutMs);

        private:

            // Thread's entry point.
            static void ThreadEntryPoint(void* data);

            // Thread's handle.
            HANDLE m_threadHandle;

            // Action to perform.
            const std::function<void()> m_action;
        };


        ThreadAction::ThreadAction(const std::function<void()> action)
            : m_action(action)
        {
            DWORD threadId;
            m_threadHandle = CreateThread(
                0,              // Security attributes
                0,              // Stack size
                (LPTHREAD_START_ROUTINE)ThreadEntryPoint,
                this,
                0,              // Creation flags
                &threadId);

            TestAssert(m_threadHandle != nullptr, "Error: failed to start thread.");
        }


        ThreadAction::~ThreadAction()
        {
            int result = CloseHandle(m_threadHandle);
            TestAssert(result != 0, "Error closing thread handle.");
        }


        void ThreadAction::Action()
        {
            try
            {
                m_action();
            }
            catch (...)
            {
                TestFail("Unexpected exception");
            }
        }


        bool ThreadAction::WaitForCompletion(unsigned timeoutMs)
        {
            const DWORD result = WaitForSingleObject(m_threadHandle, timeoutMs);
            return result == WAIT_OBJECT_0;
        }


        void ThreadAction::ThreadEntryPoint(void* data)
        {
            ThreadAction* thread = static_cast<ThreadAction*>(data);
            thread->Action();
        }


        TestCase(WaitForCompletionTest)
        {
            static const SerialNumber c_anyCutoffSerialNumber = 10;
            static const unsigned c_waitForCompletionTimeoutInMS = 200;

            TokenTracker tracker(c_anyCutoffSerialNumber, 2);

            ThreadAction getAndHoldToken1([&]() {
                Sleep(100);
                tracker.OnTokenComplete(c_anyCutoffSerialNumber - 1);
            });

            ThreadAction getAndHoldToken2([&]() {
                Sleep(150);
                tracker.OnTokenComplete(c_anyCutoffSerialNumber - 2);
            });

            const bool hasCompleted = tracker.WaitForCompletion(c_waitForCompletionTimeoutInMS);
            TestAssert(hasCompleted);

            // Threads holding the tokens should be finished by now.
            TestAssert(getAndHoldToken1.WaitForCompletion(1));
            TestAssert(getAndHoldToken2.WaitForCompletion(1));
        }
   }
}
