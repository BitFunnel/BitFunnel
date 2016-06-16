#include <limits>       // For ULONG_MAX.

#include "LoggerInterfaces/Logging.h"
#include "TokenTracker.h"

namespace BitFunnel
{
    TokenTracker::TokenTracker(SerialNumber cutoffSerialNumber, 
                               unsigned remainingTokenCount)
        : m_cutoffSerialNumber(cutoffSerialNumber),
          m_remainingTokenCount(remainingTokenCount)
    {
    }


    TokenTracker::~TokenTracker()
    {
    }


    bool TokenTracker::OnTokenComplete(SerialNumber serialNumber)
    {
        // Until a tracker declares itself "complete" by returning true from
        // this method, it will be continuously called even for serial numbers
        // greater or equal to the cutoff value. This is because tokens issued
        // later (with greater serial number) may be returned earlier than the
        // "older" ones. TokenManager does not sort out returned tokens, and 
        // simply calls all of its registered trackers. Therefore it is 
        // important to compare the serial number with the cutoff value.
        if (serialNumber < m_cutoffSerialNumber)
        {

            unsigned newValue;
            // This lock is to prevent a race between notification on the
            // condition variable and waiting on the condition varaible.
            {
                std::lock_guard<std::mutex> lock(m_conditionLock);
                newValue = --m_remainingTokenCount;
            }

            // m_remainingTokenCount should always be >= 0 because we know the
            // number of tokens in flight when starting the tracker.
            // Decrementing 0 would result in an underflow.
            LogAssertB(newValue != std::numeric_limits<unsigned int>::max(),"");

            if (newValue == 0)
            {
                m_condition.notify_all();
                return true;
            }
            else
            {
                return false;
            }
        }

        // A token which is not of our interest cannot make tracking complete.
        return false;
    }


    bool TokenTracker::IsComplete() const
    {
        return m_remainingTokenCount.load() == 0;
    }


    // TODO: does this need a timeout? It would be easy to add since we're
    // using a std::condition_variable, which has a method with a built-in
    // timeout
    void TokenTracker::WaitForCompletion()
    {
        std::unique_lock<std::mutex> lock(m_conditionLock);
        while (!IsComplete())
        {
            m_condition.wait(lock);
        }
    }
}
