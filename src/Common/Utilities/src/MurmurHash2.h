//
// MurmurHash2 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.
//
#pragma once

namespace BitFunnel
{
    // 64-bit hash for 64-bit platforms
    unsigned __int64 MurmurHash64A(const void *key, size_t len, unsigned seed);

    // 32-bit hash for 64-bit platforms
    unsigned __int32 MurmurHash32A(const void *key, size_t len, unsigned seed);
}
