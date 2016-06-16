#include "stdafx.h"

#include <algorithm>

#include "BitFunnel/Factories.h"
#include "LockGuard.h"
#include "LoggerInterfaces/Logging.h"
#include "TokenManager.h"
#include "TokenTracker.h"

namespace BitFunnel
{
    // Timeout to wait for outstanding tokens to be returned when TokenManager
    // is shutting down. For the average amount of time that the tokens are 
    // being held, this timeout should be more than sufficient.
    static const unsigned c_shutdownTimeoutInMs = 1 * 1000;

    std::unique_ptr<ITokenManager> Factories::CreateTokenManager()
    {
        return std::unique_ptr<ITokenManager>(new TokenManager());
    }


    TokenManager::TokenManager()
        : m_isTokenDistributionAllowed(true),
          m_tokensInFlight(0),
          m_nextSerialNumber(0),
          m_isShuttingDown(false),
          m_tokenHaveShutdown(CreateEvent(nullptr, true, false, nullptr))
    {
    }


    TokenManager::~TokenManager()
    {
        CloseHandle(m_tokenHaveShutdown);

        // This happens when the user has not called Shutdown when destroying
        // this object.
        LogAssertB(m_isShuttingDown);
    }


    void TokenManager::DisableNewTokens()
    {
        LockGuard lock(m_lock);

        m_isTokenDistributionAllowed = false;
    }


    void TokenManager::EnableNewTokens()
    {
        LockGuard lock(m_lock);

        m_isTokenDistributionAllowed = true;
    }


    Token TokenManager::RequestToken()
    {
        // Distribution of tokens will be disabled rarely and only for a short 
        // period of time (in the order of milliseconds). When it is disabled, 
        // it is sufficient to spin and wait for it to be re-enabled again.
        for (;;)
        {
            LockGuard lock(m_lock);

            LogAssertB(!m_isShuttingDown);

            if (m_isTokenDistributionAllowed)
            {
                const SerialNumber serialNumber = m_nextSerialNumber++;
                m_tokensInFlight++;
                return Token(*this, serialNumber);
            }

            // TODO: Check if we need to sleep for a short time so that we 
            // don't create thread contention on threads that are waiting for 
            // a Token.
        }
    }


    const std::shared_ptr<ITokenTracker> TokenManager::StartTracker()
    {
        LockGuard lock(m_lock);

        std::shared_ptr<TokenTracker> tracker(
            new TokenTracker(m_nextSerialNumber, m_tokensInFlight));

        // If there are no tokens in flight, then the tracker is already
        // complete. There is no point in adding it to the list of active
        // trackers. Returns it to the client so that they can check its
        // completion status, but from the TokenManager's perspective, this 
        // tracker is not of interest.
        if (!(tracker->IsComplete()))
        {
            m_trackers.push_back(tracker);
        }

        return tracker;
    }


    void TokenManager::Shutdown()
    {
        // Wait for existing tokens to be returned.
        unsigned tokensInFlightCount = 0;
        {
            LockGuard lock(m_lock);
            m_isShuttingDown = true;
            tokensInFlightCount = m_tokensInFlight;
        }

        if (tokensInFlightCount > 0)
        {
            const DWORD result = WaitForSingleObject(m_tokenHaveShutdown, 
                                                     c_shutdownTimeoutInMs);
            LogAssertB(result == WAIT_OBJECT_0);
        }
    }


    void TokenManager::OnTokenComplete(SerialNumber serialNumber)
    {
        LockGuard lock(m_lock);

        LogAssertB(m_tokensInFlight > 0);

        m_tokensInFlight--;

        // m_trackers consists of a sequence of zero or more complete trackers,
        // followed by a sequence of trackers that have not completed.

        // First notify and remove the completed trackers at the head of m_trackers.
        while (!m_trackers.empty() && m_trackers.front()->OnTokenComplete(serialNumber))
        {
            m_trackers.pop_front();
        }

        // Then notify the remaining trackers, starting with index 1, since we 
        // have already notified the first one in the while loop above.
        for (unsigned i = 1; i < m_trackers.size(); ++i)
        {
            LogAssertB(!m_trackers[i]->OnTokenComplete(serialNumber));
        }
    
        if (m_tokensInFlight == 0 && m_isShuttingDown)
        {
            SetEvent(m_tokenHaveShutdown);
        }
    }
}