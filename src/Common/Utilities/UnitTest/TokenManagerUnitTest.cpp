#include <chrono>
#include <thread>

#include "gtest/gtest.h"

#include "BitFunnel/Utilities/Factories.h"
#include "BitFunnel/Utilities/IThreadManager.h"
#include "LoggerInterfaces/ConsoleLogger.h"
#include "LoggerInterfaces/Logging.h"
#include "TokenManager.h"
#include "TokenTracker.h"

namespace BitFunnel
{
    namespace TokenManagerUnitTest
    {
        TEST(BasicTest, Trivial)
        {
            TokenManager tokenManager;

            {
                const Token token1 = tokenManager.RequestToken();
                ASSERT_EQ(token1.GetSerialNumber(), 0u);

                const Token token2 = tokenManager.RequestToken();
                ASSERT_EQ(token2.GetSerialNumber(), 1u);
            }

            tokenManager.Shutdown();
        }


        TEST(StartTrackerTest, Trivial)
        {
            TokenManager tokenManager;

            // Starting a tracker when there are no tokens in flight.
            const std::shared_ptr<ITokenTracker> noTokensTracker 
                = tokenManager.StartTracker();
            
            ASSERT_TRUE(noTokensTracker->IsComplete());

            // Tracker with token[0] in flight.
            std::shared_ptr<ITokenTracker> token0Tracker;
            {
                const Token token0 = tokenManager.RequestToken();
                ASSERT_TRUE(noTokensTracker->IsComplete());

                token0Tracker = tokenManager.StartTracker();
                ASSERT_TRUE(!(token0Tracker->IsComplete()));
            }

            // token0 goes out of scope, which should make the token0Tracker 
            // complete.
            ASSERT_TRUE(token0Tracker->IsComplete());

            // No impact on noTokensTracker.
            ASSERT_TRUE(noTokensTracker->IsComplete());

            // Tracker with token[1] in flight.
            std::shared_ptr<ITokenTracker> token1Tracker;
            {
                const Token token1 = tokenManager.RequestToken();

                token1Tracker = tokenManager.StartTracker();
                ASSERT_TRUE(!(token1Tracker->IsComplete()));

                // Request one more token after a tracker has been started and
                // release it before releasing a previous token. It should not
                // impact the tracker because this token is not of its interest.
                {
                    const Token token2 = tokenManager.RequestToken();
                    ASSERT_TRUE(!(token1Tracker->IsComplete()));
                }

                // token1Tracker is still not complete since token1 has not 
                // been returned.
                ASSERT_TRUE(!(token1Tracker->IsComplete()));
            }

            // token1 goes out of scope marking token1Tracker complete.
            ASSERT_TRUE(token1Tracker->IsComplete());

            // No impact on previous trackers.
            ASSERT_TRUE(noTokensTracker->IsComplete());
            ASSERT_TRUE(token0Tracker->IsComplete());

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

            bool OnTokenComplete(SerialNumber serialNumber);

            //
            // ITokenTracker API.
            //
            virtual bool IsComplete() const override;
            virtual void WaitForCompletion() override;

        private:
            TokenTracker m_tracker;
            const SerialNumber m_cutoffSerialNumber;
            std::atomic<bool> m_isExpectingTokens;
        };


        TestTokenTracker::TestTokenTracker(SerialNumber cutoffSerialNumber, unsigned remainingTokenCount)
            : m_tracker(cutoffSerialNumber, remainingTokenCount),
              m_cutoffSerialNumber(cutoffSerialNumber),
              m_isExpectingTokens(true)
        {
        }


        bool TestTokenTracker::OnTokenComplete(SerialNumber serialNumber)
        {
            // DESIGN NOTE: we can't ASSERT_TRUE here because that causes a void
            // return, which conflicts with this functions type.
            const bool isExpectingCallbacks = m_isExpectingTokens.load() != 0;
            EXPECT_TRUE(isExpectingCallbacks)
                << serialNumber
                << "was not expected. Cutoff: "
                << m_cutoffSerialNumber;

            const bool hasTrackingComplete = m_tracker.OnTokenComplete(serialNumber);
            if (hasTrackingComplete)
            {
                m_isExpectingTokens = false;
            }

            return hasTrackingComplete;
        }


        bool TestTokenTracker::IsComplete() const
        {
            return m_tracker.IsComplete();
        }


        void TestTokenTracker::WaitForCompletion()
        {
            return m_tracker.WaitForCompletion();
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
            }
        }


        void TokenTrackerThread::TrackTokens() const
        {
            const std::shared_ptr<ITokenTracker> tracker = m_tokenManager.StartTracker();

            // There are 2 holders of the tracker - token manager and this 
            // function.
            const long useCount = tracker.use_count();
            ASSERT_EQ(useCount, 2);

            // static const unsigned c_trackerCompletionTimeoutInMS = 1000; TODO
            tracker->WaitForCompletion();

            // Give the manager some time to de-register a tracker.
            // This is to accommodate the fact that WaitForCompletion() call 
            // here and token manager checking the status of the tracker 
            // happening on different threads and potentially one can be 
            // a little faster than the other. 

            // The tracker should now be de-registered from the token manager,
            // and this should reduce the use count by 1.
            ASSERT_EQ(tracker.use_count(), useCount - 1);

            // LogB(Logging::Info, "TokenTrackerThread", "Tracker finished in %f", timer.ElapsedTime()); TODO
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

            // static const unsigned c_threadDrainTimeout = 1000; TODO
            m_threadManager->WaitForThreads();
        }


        // Test which verifies distribution of tokens and starting/stopping
        // token trackers.
        TEST(TokenManagerWithTrackersTest, Trivial)
        {
            static const unsigned c_threadCount = 16;
            // static const unsigned c_testDuration = 5 * 1000; TODO

            TokenManager tokenManager;

            std::unique_ptr<Logging::ILogger> logger(new Logging::ConsoleLogger());
            Logging::RegisterLogger(logger.get());

            TokenThreadHolder tokenDistributor(c_threadCount, tokenManager);

            volatile bool isRunning = true;

            // Add a thread that periodically starts tracking tokens.
            TokenTrackerThread * trackerThread = new TokenTrackerThread(tokenManager, isRunning);
            tokenDistributor.AddThread(trackerThread);

            tokenDistributor.Start();

            // DESIGN NOTE: this test used to have random sleeps in the methods
            // and a 5 second sleep here. All the sleeps were removed because we
            // still get some randomness in ordering and this test shouldn't
            // take 5+ seconds to run. However, it's possible that's too extreme
            // and we should have some random sleeps.
            // However, if we end up having tests that sleep for any significant
            // length of time, we should really have that be part of some
            // more comprehensive test suite, not part of a test that's run
            // once whenever people happen to run unit tests.
            // Additionally, if we really want to check that this works we
            // should do model checking over possible re-orderings, since
            // random sleeps have a tendency to pile up and wake up in ways
            // that make them less random than we want.

            isRunning = false;
            tokenDistributor.Stop();

            {
                const Token lastToken = tokenManager.RequestToken();

                // Sanity check that some tokens have actually been issued.
                ASSERT_TRUE(lastToken.GetSerialNumber() > 0);

                LogB(Logging::Info, 
                     "TokenManagerWithTrackersTest", 
                     "Last serial num: %u", 
                     lastToken.GetSerialNumber());
            }

            tokenManager.Shutdown();
        }
    }
}
