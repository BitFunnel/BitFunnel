#pragma once

#include "BitFunnel/NonCopyable.h"


namespace BitFunnel
{
    class IThreadBase : NonCopyable
    {
    public:
        virtual ~IThreadBase() {};

        virtual void EntryPoint() = 0;
    };


    class IThreadManager
    {
    public:
        virtual ~IThreadManager() {};

        // Waits a specified amount of time for threads to exit. Returns true if all threads
        // exited successfully before the timeout period expired.
        virtual bool WaitForThreads(int timeoutInMs) = 0;
    };
}
