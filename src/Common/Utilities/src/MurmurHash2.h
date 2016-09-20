//
// MurmurHash2 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.
//
#pragma once

#include <inttypes.h>
#include <stddef.h>


namespace BitFunnel
{
    // 64-bit hash for 64-bit platforms
    uint64_t MurmurHash64A(const void *key, size_t len, unsigned seed);
}
