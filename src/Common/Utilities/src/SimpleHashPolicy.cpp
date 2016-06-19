#include "stdafx.h"

#include <Windows.h>

#include "LoggerInterfaces/Logging.h"
#include "SimpleHashPolicy.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // SimpleHashPolicy::Threadsafe
    //
    //*************************************************************************
    SimpleHashPolicy::Threadsafe::Threadsafe(bool allowResize)
        : m_allowsResize(false)
    {
        LogAssertB(!allowResize);
    }


    bool SimpleHashPolicy::Threadsafe::UpdateKeyIfUnchanged(volatile unsigned __int64* keys,
                                                            unsigned slot,
                                                            unsigned __int64 expectedCurrentKey,
                                                            unsigned __int64 desiredKey)
    {
        // Only perform the operation if the existing key has the expected existing value (no other threads touch it).
        return (static_cast<LONGLONG>(expectedCurrentKey) == InterlockedCompareExchange64(reinterpret_cast<volatile LONGLONG*>(keys + slot),
                                                                                          static_cast<LONGLONG>(desiredKey),
                                                                                          static_cast<LONGLONG>(expectedCurrentKey)));
    }


    bool SimpleHashPolicy::Threadsafe::AllowsResize() const
    {
        return m_allowsResize;
    }


    //*************************************************************************
    //
    // SimpleHashPolicy::SingleThreaded
    //
    //*************************************************************************
    SimpleHashPolicy::SingleThreaded::SingleThreaded(bool allowResize)
        : m_allowsResize(allowResize)
    {
    }


    bool SimpleHashPolicy::SingleThreaded::UpdateKeyIfUnchanged(volatile unsigned __int64* keys,
                                                                unsigned slot,
                                                                unsigned __int64 expectedCurrentKey,
                                                                unsigned __int64 desiredKey)
    {
        LogAssertB(keys[slot] == expectedCurrentKey);

        keys[slot] = desiredKey;
        return true;
    }


    bool SimpleHashPolicy::SingleThreaded::AllowsResize() const
    {
        return m_allowsResize;
    }
}
