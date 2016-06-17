#pragma once

#include <condition_variable> // Uses std::contiion_variable.
#include <deque>              // Uses std::deque.
#include <memory>             // Uses std::shared_ptr.
#include <mutex>              // Uses std::mutex.

#include "Token.h"  // Inherits from ITokenManager and ITokenListener

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
        unsigned m_nextSerialNumber;

        // Number of tokens currently in-flight.
        unsigned m_tokensInFlight;

        // Flag indicating that TokenManager is shutting down.
        bool m_isShuttingDown;

        // A list of registered token trackers.
        // DESIGN NOTE: std::deque is chosen since we always want to add new 
        // trackers at the back and remove the completed ones off the front.
        // The trackers in front will always complete faster than the ones
        // at the back - since they were started earlier - and we need to 
        // be able to pop the trackers off the list as soon as they complete.
        std::deque<std::shared_ptr<TokenTracker>> m_trackers;

        // Lock protecting all of the above private members to ensure 
        // thread safety.
        std::mutex m_lock;

        // Lock on condition variable.
        std::mutex m_condLock;

        // Signal that all tokens have been destroyed. This allows Shutdown()
        // to proceed.
        std::condition_variable m_condition;
    };
}
