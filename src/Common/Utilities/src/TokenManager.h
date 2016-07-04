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


#pragma once

#include <atomic>             // Uses std::atomic.
#include <condition_variable> // Uses std::contiion_variable.
#include <deque>              // Uses std::deque.
#include <memory>             // Uses std::shared_ptr.
#include <mutex>              // Uses std::mutex.

#include "BitFunnel/Token.h"  // Inherits from ITokenManager and ITokenListener

namespace BitFunnel
{
    class TokenTracker;

    //*************************************************************************
    //
    // TokenManager provides an implementation of ITokenManager which assists
    // in tracking owners of tokens issued at a particular cut off point, as
    // well as to stop and resume distributing new tokens.
    // This class is thread-safe.
    //
    // DESIGN NOTE: To make it easier to track old tokens, all tokens are
    // assigned a monotonically increasing serial numbers. With this scheme
    // ITokenTracker needs only store the most recently issued SerialNumber and
    // the number of Tokens currently in existence.
    //
    //*************************************************************************
    class TokenManager : public ITokenManager,
                         private ITokenListener
    {
    public:
        TokenManager();

        ~TokenManager();

        //
        // ITokenManager API.
        //

        virtual Token RequestToken() override;
        virtual const std::shared_ptr<ITokenTracker> StartTracker() override;
        virtual void Shutdown() override;

    private:

        //
        // ITokenListener API.
        //
        virtual void OnTokenComplete(SerialNumber serialNumber) override;

        // Serial number for the next issued token.
        std::atomic<unsigned> m_nextSerialNumber;

        // Number of tokens currently in-flight.
        std::atomic<unsigned> m_tokensInFlight;

        // Flag indicating that TokenManager is shutting down.
        std::atomic<bool> m_isShuttingDown;

        // A list of registered token trackers.
        // DESIGN NOTE: std::deque is chosen since we always want to add new
        // trackers at the back and remove the completed ones off the front.
        // The trackers in front will always complete faster than the ones
        // at the back - since they were started earlier - and we need to
        // be able to pop the trackers off the list as soon as they complete.
        std::deque<std::shared_ptr<TokenTracker>> m_trackers;

        // This lock is required to protect two things. First, it's used
        // to protect the m_trackers deque, which isn't thread safe.
        // Second, it's used to protect the tuple (m_nextSerialNumber,
        // m_tokensInFlight), which must be updated in tandem.
        // Making each of those atomic isn't sufficient. because a new tracker
        // instantiated when only one of the two variables has been updated will
        // have an inconsistent state.
        // This mutex is also used to protect the condition variable, although,
        // strictly speaking, a finer grained mutex could be used that only
        // protects the condition variable and m_tokensInFlight.
        std::mutex m_lock;

        // Signal that all tokens have been destroyed. This allows Shutdown()
        // to proceed.
        std::condition_variable m_shutdownCondition;
    };
}
