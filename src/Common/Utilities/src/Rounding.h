#pragma once

#include <stddef.h>  // For size_t.

namespace BitFunnel
{   
    // Get the nearest power of 2 integer for valueToBeRounded.
    // For example, for 17, it should return 16, and for 30, it should return 32.
    size_t RoundToTheNearestPowerOf2(size_t valueToBeRounded);

    // Get the nearest power of 2 integer for valueToBeRounded which is greater than
    // valueToBeRounded.
    // For example, for 17, it should return 32, and for 30, it should return 32.
    size_t RoundUpPowerOf2(size_t valueToBeRounded);

    // Rounds up the requested size to the next multiple of 
    // roundUpAlignment.
    size_t RoundUp(size_t requestedSize, size_t roundUpAlignment);
}
