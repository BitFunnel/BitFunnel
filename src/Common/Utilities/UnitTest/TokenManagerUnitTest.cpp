#include "stdafx.h"

#include <functional>

#include "BitFunnel/Factories.h"
#include "BitFunnel/IThreadManager.h"
#include "BitFunnel/Stopwatch.h"
#include "LoggerInterfaces/ConsoleLogger.h"
#include "LoggerInterfaces/Logging.h"
#include "SuiteCpp/UnitTest.h"
#include "ThreadsafeState.h"
#include "TokenManager.h"
#include "TokenTracker.h"

namespace BitFunnel
{
    namespace TokenManagerUnitTest
    {
        TestCase(BasicTest)
        {
            TokenManager tokenManager;

            {
                const Token token1 = tokenManager.RequestToken();
                TestEqual(token1.GetSerialNumber(), 0);

                const Token token2 = tokenManager.RequestToken();
                TestEqual(token2.GetSerialNumber(), 1);
            }

            tokenManager.Shutdown();
        }


        TestCase(StartTrackerTest)
        {
            TokenManager tokenManager;

            // Starting a tracker when there are no tokens in flight.
            const std::shared_ptr<ITokenTracker> noTokensTracker 
                = tokenManager.StartTracker();
            
            TestAssert(noTokensTracker->IsComplete());

            // Tracker with token[0] in flight.
            std::shared_ptr<ITokenTracker> token0Tracker;
            {
                const Token token0 = tokenManager.RequestToken();
                TestAssert(noTokensTracker->IsComplete());

                token0Tracker = tokenManager.StartTracker();
                TestAssert(!(token0Tracker->IsComplete()));
            }

            // token0 goes out of scope, which should make the token0Tracker 
            // complete.
            TestAssert(token0Tracker->IsComplete());

            // No impact on noTokensTracker.
            TestAssert(noTokensTracker->IsComplete());

            // Tracker with token[1] in flight.
            std::shared_ptr<ITokenTracker> token1Tracker;
            {
                const Token token1 = tokenManager.RequestToken();

                token1Tracker = tokenManager.StartTracker();
                TestAssert(!(token1Tracker->IsComplete()));

                // Request one more token after a tracker has been started and
                // release it before releasing a previous token. It should not
                // impact the tracker because this token is not of its interest.
                {
                    const Token token2 = tokenManager.RequestToken();
                    TestAssert(!(token1Tracker->IsComplete()));
                }

                // token1Tracker is still not complete since token1 has not 
                // been returned.
                TestAssert(!(token1Tracker->IsComplete()));
            }

            // token1 goes out of scope marking token1Tracker complete.
            TestAssert(token1Tracker->IsComplete());

            // No impact on previous trackers.
            TestAssert(noTokensTracker->IsComplete());
            TestAssert(token0Tracker->IsComplete());

            tokenManager.Shutdown();
        }


        //*********************************************************************
        //
        // Test implementation of ITokenTracker that knows when it is supposed
        // to receive token confirmations and when not.
        //
        //*********************************************************************
        class TestTokenTracker : public ITokenTracker
        {
        public:
            TestTokenTracker(SerialNumber cutoffSerialNumber, unsigned remainingTokenCount);

            // Instructs the tracker that from now on it should not receive token 
            // notifications, and if it does, it will be considered a test failure.
            void StopExpectingTokens();

            bool OnTokenComplete(SerialNumber serialNumber);

            //
            // ITokenTracker API.
            //
            virtual bool IsComplete() const override;
            virtual bool WaitForCompletion(unsigned timeoutInMs) override;

        private:
            TokenTracker m_tracker;
            ThreadsafeState m_isExpectingTokens;
            const SerialNumber m_cutoffSerialNumber;
        };


        TestTokenTracker::TestTokenTracker(SerialNumber cutoffSerialNumber, unsigned remainingTokenCount)
            : m_tracker(cutoffSerialNumber, remainingTokenCount),
              m_cutoffSerialNumber(cutoffSerialNumber),
              m_isExpectingTokens(1)
        {
        }


        void TestTokenTracker::StopExpectingTokens()
        {
            for (unsigned attemptsLeft = 10; attemptsLeft > 0; --attemptsLeft)
            {
                if (m_isExpectingTokens.TryThreadsafeTransit(1, 0))
                {
                    return;
                }
            }

            TestFail("Could not stop expecting tracking after 10 attempts");
        }


        bool TestTokenTracker::OnTokenComplete(SerialNumber serialNumber)
        {
            const bool isExpectingCallbacks = m_isExpectingTokens.ThreadsafeGetState() != 0;
            TestAssert(isExpectingCallbacks, 
                       "Token %u was not expected. Cutoff serial number: %u", 
                       serialNumber, 
                       m_cutoffSerialNumber);
            
            const bool hasTrackingComplete = m_tracker.OnTokenComplete(serialNumber);
            if (hasTrackingComplete)
            {
                StopExpectingTokens();
            }

            return hasTrackingComplete;
        }


        bool TestTokenTracker::IsComplete() const
        {
            return m_tracker.IsComplete();
        }


        bool TestTokenTracker::WaitForCompletion(unsigned timeoutInMs)
        {
            return m_tracker.WaitForCompletion(timeoutInMs);
        }


        //*********************************************************************
        //
        // Represents a thread which gets a token from a token manager, holds it 
        // for a random amount of time, and releases it. 
        //
        //*********************************************************************
        class TokenRequestorThread : public IThreadBase
        {
        public:

            TokenRequestorThread(ITokenManager& tokenManager,
                                 volatile bool& isRunning);

            virtual void EntryPoint() override;

        private:
            void GetAndReleaseToken() const;

            // Parent token manager from which to request a token.
            ITokenManager& m_tokenManager;

            // Flag to indicate if the thread should be running.
            volatile bool& m_isRunning;
        };


        TokenRequestorThread::TokenRequestorThread(ITokenManager& tokenManager,
                                                   volatile bool& isRunning)
            : m_tokenManager(tokenManager),
              m_isRunning(isRunning)
        {
        }


        void TokenRequestorThread::GetAndReleaseToken() const
        {
            const Token token = m_tokenManager.RequestToken();
            Sleep(10 + rand() % 20);
        }


        void TokenRequestorThread::EntryPoint()
        {
            while (m_isRunning)
            {
                GetAndReleaseToken();
            }
        }


        //*********************************************************************
        //
        // TokenTrackerThread - represent a thread which periodically starts a
        // new token tracker at an arbitrary time and waits for its completion.
        //
        //*********************************************************************
        class TokenTrackerThread : public IThreadBase
        {
        public:
            TokenTrackerThread(ITokenManager& tokenManager,
                               volatile bool& isRunning);

            ~TokenTrackerThread() {};

            virtual void EntryPoint() override;

        private:

            // Starts a new tracker and waits for its completion.
            void TrackTokens() const;

            ITokenManager& m_tokenManager;
            volatile bool& m_isRunning;
        };


        TokenTrackerThread::TokenTrackerThread(ITokenManager& tokenManager,
                                               volatile bool& isRunning)
            : m_tokenManager(tokenManager),
              m_isRunning(isRunning)
        {
        }


        void TokenTrackerThread::EntryPoint()
        {
            while (m_isRunning)
            {
                TrackTokens();

                Sleep(500 + rand() % 50);
            }
        }


        void TokenTrackerThread::TrackTokens() const
        {
            Stopwatch timer;
            const std::shared_ptr<ITokenTracker> tracker = m_tokenManager.StartTracker();

            // There are 2 holders of the tracker - token manager and this 
            // function.
            const long useCount = tracker.use_count();
            TestEqual(useCount, 2);

            static const unsigned c_trackerCompletionTimeoutInMS = 1000;
            const bool hasTrackerCompleted = 
                tracker->WaitForCompletion(c_trackerCompletionTimeoutInMS);

            TestAssert(hasTrackerCompleted);

            // Give the manager some time to de-register a tracker.
            // This is to accommodate the fact that WaitForCompletion() call 
            // here and token manager checking the status of the tracker 
            // happening on different threads and potentially one can be 
            // a little faster than the other. 
            Sleep(10);

            // The tracker should now be de-registered from the token manager,
            // and this should reduce the use count by 1.
            TestEqual(tracker.use_count(), useCount - 1);

            LogB(Logging::Info, "TokenTrackerThread", "Tracker finished in %f", timer.ElapsedTime());
        }


        //*********************************************************************
        //
        // TokenThreadHolder - class which simulates multiple threads which 
        // request and return tokens from a manager, and optionally, a thread(s)
        // which periodically asks the token manager to start tracking tokens
        // at an arbitraty time.
        //
        //*********************************************************************
        class TokenThreadHolder
        {
        public:
            TokenThreadHolder(unsigned threadCount, ITokenManager& tokenManager);

            ~TokenThreadHolder();

            void AddThread(IThreadBase* thread);

            void Start();

            void Stop();

        private:

            std::vector<IThreadBase*> m_threads;
            std::unique_ptr<IThreadManager> m_threadManager;
            volatile bool m_isRunning;
        };


        TokenThreadHolder::TokenThreadHolder(unsigned threadCount, ITokenManager& tokenManager)
            : m_isRunning(true)
        {
            m_threads.resize(threadCount);

            for (unsigned i = 0; i < threadCount; ++i)
            {
                m_threads[i] = new TokenRequestorThread(tokenManager, m_isRunning);
            }
        }


        TokenThreadHolder::~TokenThreadHolder()
        {
            for (const auto& thread: m_threads)
            {
                delete thread;
            }
        }


        void
        TokenThreadHolder::AddThread(IThreadBase* thread)
        {
            m_threads.push_back(thread);
        }


        void TokenThreadHolder::Start()
        {
            m_threadManager.reset(Factories::CreateThreadManager(m_threads));
        }


        void TokenThreadHolder::Stop()
        {
            m_isRunning = false;

            static const unsigned c_threadDrainTimeout = 1000;
            const bool haveThreadsStopped = m_threadManager->WaitForThreads(c_threadDrainTimeout);

            TestEqual(haveThreadsStopped, true);
        }


        // Test which verifies distribution of tokens and starting/stopping
        // token trackers.
        TestCase(TokenManagerWithTrackersTest)
        {
            static const unsigned c_threadCount = 16;
            static const unsigned c_testDuration = 5 * 1000;

            TokenManager tokenManager;

            std::unique_ptr<Logging::ILogger> logger(new Logging::ConsoleLogger());
            Logging::RegisterLogger(logger.get());

            TokenThreadHolder tokenDistributor(c_threadCount, tokenManager);

            volatile bool isRunning = true;

            // Add a thread that periodically starts tracking tokens.
            TokenTrackerThread * trackerThread = new TokenTrackerThread(tokenManager, isRunning);
            tokenDistributor.AddThread(trackerThread);

            tokenDistributor.Start();

            Sleep(c_testDuration);

            isRunning = false;
            tokenDistributor.Stop();

            {
                const Token lastToken = tokenManager.RequestToken();

                // Sanity check that some tokens have actually been issued.
                TestAssert(lastToken.GetSerialNumber() > 0);

                LogB(Logging::Info, 
                     "TokenManagerWithTrackersTest", 
                     "Last serial num: %u", 
                     lastToken.GetSerialNumber());
            }

            tokenManager.Shutdown();
        }


        // Class which represent an action which needs to be performed on a 
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


        // Test which verifies disabling/enabling distribution of tokens.
        TestCase(TokenManagerDisableTokensTest)
        {
            TokenManager tokenManager;

            // Maximum time in milliseconds in which we are expected to get a token.
            static const unsigned c_requestTokenMaxTime = 100;

            // Maximum time in milliseconds to wait for a thread to finish.
            static const unsigned c_threadCompletionTimeout = 10 * 1000;

            // Gets a single token, check if time taken is within acceptable 
            // boundaries.
            {
                Stopwatch timer;
                ThreadAction thread1([&] () {
                    tokenManager.RequestToken();
                });

                TestAssert(thread1.WaitForCompletion(c_threadCompletionTimeout));

                const double elapsedTime = timer.ElapsedTime();
                TestAssert(elapsedTime * 1000 < c_requestTokenMaxTime);
            }

            // Gets 2 tokens in multiple threads, check if time taken for each 
            // is within acceptable boundaries.
            {
                Stopwatch timer;
                double elapsedTime1, elapsedTime2;

                ThreadAction thread1([&] () {
                    tokenManager.RequestToken();
                    elapsedTime1 = timer.ElapsedTime();
                });


                ThreadAction thread2([&] () {
                    tokenManager.RequestToken();
                    elapsedTime2 = timer.ElapsedTime();
                });

                TestAssert(thread1.WaitForCompletion(c_threadCompletionTimeout));
                TestAssert(thread2.WaitForCompletion(c_threadCompletionTimeout));

                TestAssert(elapsedTime1 < c_requestTokenMaxTime);
                TestAssert(elapsedTime2 < c_requestTokenMaxTime);
            }

            // Launches a thread which needs to get a token when distribution of
            // tokens is disabled. Re-enable distribution of tokens after a 
            // defined period of time. Checks if the token can now be obtained 
            // and that it indeed was obtained only after re-enabling of tokens.
            {
                Stopwatch timer;

                tokenManager.DisableNewTokens();

                static const unsigned c_disabledTokensDuration = 5000;

                ThreadAction thread1([&] () {
                    tokenManager.RequestToken();
                });

                Sleep(c_disabledTokensDuration);

                tokenManager.EnableNewTokens();

                TestAssert(thread1.WaitForCompletion(c_threadCompletionTimeout));
                const double elapsedTime1 = timer.ElapsedTime();

                TestAssert(elapsedTime1 * 1000 >= c_disabledTokensDuration, 
                           "We were not expected to get a token in %f seconds, but got one after %f seconds",
                           c_disabledTokensDuration / 1000.0,
                           elapsedTime1);
                TestAssert(elapsedTime1 * 1000 < c_disabledTokensDuration + c_requestTokenMaxTime,
                           "We were expected to get a token in %f seconds, but got one after %f seconds",
                           (c_disabledTokensDuration + c_requestTokenMaxTime) / 1000.0,
                           elapsedTime1);
            }

            tokenManager.Shutdown();
        }


        void GetAndHoldToken(ITokenManager& tokenManager, unsigned durationInMs)
        {
            const Token token = tokenManager.RequestToken();
            Sleep(durationInMs);
        }


        TestCase(ShutdownTest)
        {
            std::unique_ptr<TokenManager> tokenManager(new TokenManager());

            bool thread1HasStarted = false;
            bool thread1HasExited = false;
            ThreadAction thread1([&] () {
                thread1HasStarted = true;

                GetAndHoldToken(*tokenManager, 100);

                thread1HasExited = true;
            });

            bool thread2HasStarted = false;
            bool thread2HasExited = false;
            ThreadAction thread2([&] () {
                thread2HasStarted = true;

                GetAndHoldToken(*tokenManager, 100);

                thread2HasExited = true;
            });

            // Give threads a chance to get tokens. Check that threads have 
            // started but not yet exited.
            Sleep(50);
            TestAssert(thread1HasStarted);
            TestAssert(thread2HasStarted);
            TestAssert(!thread1HasExited);
            TestAssert(!thread2HasExited);

            tokenManager->Shutdown();

            // Resetting unique_ptr, effectively asking TokenManager to shutdown.
            tokenManager.reset();

            TestAssert(thread1HasExited);
            TestAssert(thread2HasExited);
        }
    }
}