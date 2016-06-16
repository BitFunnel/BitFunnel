#include "stdafx.h"

#include <limits>       // For ULONG_MAX.
#include <Windows.h>    // For Sleep()

#include "BitFunnel/Stopwatch.h"
#include "LoggerInterfaces/Logging.h"
#include "TokenTracker.h"

namespace BitFunnel
{
    // When polling for completion, this class will sleep this long between
    // poll iterations. 1 is the minimal amount that can be provided, and 
    // will work for scenarios that this class provides. At the same time,
    // sleeping is essential so that we give the other threads a slice of
    // CPU time.
    static const unsigned c_pollingSleepIntervalInMs = 1;

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
            const unsigned newValue = m_remainingTokenCount.ThreadsafeDecrement();

            // m_remainingTokenCount should always be positive in this path
            // because we know an accurate number of tokens in flight when
            // starting a tracker, but we want to assert on it. Because 
            // m_remainingTokenCount is of the unsigned type, decrementing it
            // by 1 would have made it ULONG_MAX. 
            LogAssertB(newValue != ULONG_MAX);

            return newValue == 0;
        }

        // A token which is not of our interest cannot make tracking complete.
        return false;
    }


    bool TokenTracker::IsComplete() const
    {
        return m_remainingTokenCount.ThreadsafeGetValue() == 0;
    }


    bool TokenTracker::WaitForCompletion(unsigned timeoutInMs)
    {
        Stopwatch timer;
        for (;;)
        {
            if (IsComplete())
            {
                return true;
            }

            if (timer.ElapsedTime() > timeoutInMs / 1000.0)
            {
                return false;
            }

            // This is not performance critical, sleeping for a short 
            // duration is fine. For more performant options we could use 
            // events and signalling.
            Sleep(c_pollingSleepIntervalInMs);
        }
    }
}