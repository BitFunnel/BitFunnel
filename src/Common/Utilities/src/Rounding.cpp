#include "Rounding.h"


namespace BitFunnel
{   
    size_t RoundUpPowerOf2(size_t valueToBeRounded)
    {
        if (0 == valueToBeRounded)
        {
            return 1;
        }

        size_t value = valueToBeRounded;

        // The source of this algorithm is
        // http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
        value--;
        value |= value >> 1;
        value |= value >> 2;
        value |= value >> 4;
        value |= value >> 8;
        value |= value >> 16;
        value |= value >> 32;
        value++;

        return value;
    }


    size_t RoundToTheNearestPowerOf2(size_t valueToBeRounded)
    {
        const size_t nextPowerOf2 = RoundUpPowerOf2(valueToBeRounded);
        const size_t previousPowerOf2 = nextPowerOf2 >> 1;

        return (nextPowerOf2 - valueToBeRounded) >= (valueToBeRounded - previousPowerOf2)
               ? previousPowerOf2
               : nextPowerOf2;
    }


    size_t RoundUp(size_t requestedSize, size_t roundUpAlignment)
    {
        const size_t roundedValue = (requestedSize + roundUpAlignment - 1)
                                    / roundUpAlignment * roundUpAlignment;

        return roundedValue;
    }
}
