#include <algorithm>

#include "BitFunnel/Utilities/Factories.h"
#include "LoggerInterfaces/Logging.h"
#include "TokenManager.h"
#include "TokenTracker.h"

namespace BitFunnel
{
    std::unique_ptr<ITokenManager> Factories::CreateTokenManager()
    {
        return std::unique_ptr<ITokenManager>(new TokenManager());
    }


    TokenManager::TokenManager()
        : m_nextSerialNumber(0),
          m_tokensInFlight(0),
          m_isShuttingDown(false)
    {
    }


    TokenManager::~TokenManager()
    {
        // This happens when the user has not called Shutdown when destroying
        // this object.
        LogAssertB(m_isShuttingDown, "Destructed TokenManager without shutdown.");
    }


    Token TokenManager::RequestToken()
    {
        std::lock_guard<std::mutex> lock(m_lock);

        LogAssertB(!m_isShuttingDown, "Requested Token while shutting down");

        const SerialNumber serialNumber = m_nextSerialNumber++;
        ++m_tokensInFlight;
        return Token(*this, serialNumber);
    }


    const std::shared_ptr<ITokenTracker> TokenManager::StartTracker()
    {
        std::lock_guard<std::mutex> lock(m_lock);

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
            std::lock_guard<std::mutex> lock(m_lock);
            m_isShuttingDown = true;
            tokensInFlightCount = m_tokensInFlight;
        }

        // TODO: consider if we want to timeout and log an error.
        while (tokensInFlightCount > 0)
        {
            std::unique_lock<std::mutex> lock(m_condLock);
            m_condition.wait(lock);
        }
    }


    void TokenManager::OnTokenComplete(SerialNumber serialNumber)
    {
        std::lock_guard<std::mutex> lock(m_lock);

        LogAssertB(m_tokensInFlight > 0,
                   "Token completed with <= 0 tokens in flight.");

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
            LogAssertB(!m_trackers[i]->OnTokenComplete(serialNumber),
                       "Tracker completed when older tracker didn't complete.");
        }
    
        if (m_tokensInFlight == 0 && m_isShuttingDown)
        {
            m_condition.notify_all();
        }
    }
}
