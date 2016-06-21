#include "LoggerInterfaces/Logging.h"
#include "SimpleHashPolicy.h"

#ifdef _MSC_VER
#include <Windows.h>  // For InterlockedCompareExchange64.
#endif


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
        LogAssertB(!allowResize,
                   "Illegal combination: allowResize && threadsafe.");
    }


    bool SimpleHashPolicy::Threadsafe::UpdateKeyIfUnchanged(
        volatile uint64_t* keys,
        unsigned slot,
        uint64_t expectedCurrentKey,
        uint64_t desiredKey)
    {
        // Only perform the operation if the existing key has the expected
        // existing value (no other threads touch it).
#ifndef _MSC_VER
        return (static_cast<uint64_t>(expectedCurrentKey) ==
                __sync_val_compare_and_swap(reinterpret_cast<volatile uint64_t*>(keys + slot),
                                            static_cast<uint64_t>(expectedCurrentKey),
                                            static_cast<uint64_t>(desiredKey)));
#else
        return (static_cast<LONGLONG>(expectedCurrentKey) ==
                InterlockedCompareExchange64(reinterpret_cast<volatile LONGLONG*>(keys + slot),
                                             static_cast<LONGLONG>(desiredKey),
                                             static_cast<LONGLONG>(expectedCurrentKey)));
#endif
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


    bool SimpleHashPolicy::SingleThreaded::UpdateKeyIfUnchanged(
        volatile uint64_t* keys,
        unsigned slot,
        uint64_t expectedCurrentKey,
        uint64_t desiredKey)
    {
        LogAssertB(keys[slot] == expectedCurrentKey,
                   "Found changed key in UpdateKeyIfUnchanged");

        keys[slot] = desiredKey;
        return true;
    }


    bool SimpleHashPolicy::SingleThreaded::AllowsResize() const
    {
        return m_allowsResize;
    }
}
