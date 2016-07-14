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
        LogAssertB(!m_isShuttingDown, "Requested Token while shutting down");

        std::lock_guard<std::mutex> lock(m_lock);
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
        LogAssertB(!m_isShuttingDown, "Multiple shutdowns seen.\n");
        m_isShuttingDown = true;

        // Wait for existing tokens to be returned.
        // TODO: consider if we want to timeout and log an error.
        while (m_tokensInFlight > 0)
        {
            std::unique_lock<std::mutex> lock(m_lock);
            m_shutdownCondition.wait(lock);
        }
    }


    void TokenManager::OnTokenComplete(SerialNumber serialNumber)
    {
        LogAssertB(m_tokensInFlight > 0,
                   "Token completed with <= 0 tokens in flight.");

        m_lock.lock();
        --m_tokensInFlight;

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
        m_lock.unlock();

        if (m_tokensInFlight == 0 && m_isShuttingDown)
        {
            m_shutdownCondition.notify_all();
        }
    }
}
