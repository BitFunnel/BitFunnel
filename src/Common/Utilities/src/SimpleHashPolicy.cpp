#include "LoggerInterfaces/Logging.h"
#include "SimpleHashPolicy.h"

#ifdef _MSC_VER
// TODO: is this #define even necessary, or does VC++ have
// __sync_val_compare_and_swap?
#include <Windows.h>
#define __sync_val_compare_and_swap(ptr, expected, desired) InterlockedCompareExchange64(ptr, desired, expected)
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
        return (static_cast<uint64_t>(expectedCurrentKey) ==
                __sync_val_compare_and_swap(reinterpret_cast<volatile uint64_t*>(keys + slot),
                                             static_cast<uint64_t>(expectedCurrentKey),
                                             static_cast<uint64_t>(desiredKey)));
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
