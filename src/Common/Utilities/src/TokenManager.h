#pragma once

#include <deque>              // Uses std::deque.
#include <memory>             // Uses std::shared_ptr.

#include "BitFunnel/Token.h"  // Inherits from ITokenManager and ITokenListener
#include "Mutex.h"

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

        // If distribution of tokens is disabled, this method blocks until
        // distribution is re-enabled.
        virtual Token RequestToken() override;
        virtual void DisableNewTokens() override;
        virtual void EnableNewTokens() override;
        virtual const std::shared_ptr<ITokenTracker> StartTracker() override;
        virtual void Shutdown() override;

    private:

        //
        // ITokenListener API.
        //
        virtual void OnTokenComplete(SerialNumber serialNumber) override;

        // Number of tokens currently in-flight.
        unsigned m_tokensInFlight;

        // Serial number for the next issued token.
        unsigned m_nextSerialNumber;

        // Flag to indicate whether the distribution of tokens is allowed.
        bool m_isTokenDistributionAllowed;

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
        Mutex m_lock;

        // Event which is used to signal when all of the tokens in existence
        // have been destroyed when the TokenManager is shutting down.
        HANDLE m_tokenHaveShutdown;
    };
}