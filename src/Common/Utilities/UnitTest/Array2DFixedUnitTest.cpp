#include <sstream>

#include "gtest/gtest.h"

#include "Array.h"
#include "ConstructorDestructorCounter.h"


namespace BitFunnel
{
    namespace Array2DFixedUnitTest
    {
        TEST(Initialization, Trivial)
        {
            ConstructorDestructorCounter::ClearCount();

            const unsigned size1 = 4;
            const unsigned size2 = 17;

            {
                Array2DFixed<ConstructorDestructorCounter, size1, size2> a;

                EXPECT_EQ(ConstructorDestructorCounter::GetConstructorCount(), size1 * size2);
                EXPECT_EQ(ConstructorDestructorCounter::GetDestructorCount(), 0u);
            }

            EXPECT_EQ(ConstructorDestructorCounter::GetDestructorCount(), size1 * size2);
        }


        TEST(FieldAccess, Trivial)
        {
            const unsigned size1 = 3;
            const unsigned size2 = 5;

            Array2DFixed<unsigned, size1, size2> a;

            unsigned counter = 0;
            for (unsigned x = 0; x < size1; ++x)
            {
                for (unsigned y = 0; y < size2; ++y)
                {
                    a.At(x, y) =  counter++;
                }
            }

            counter = 0;
            for (unsigned x = 0; x < size1; ++x)
            {
                for (unsigned y = 0; y < size2; ++y)
                {
                    EXPECT_EQ(a.At(x, y), counter++);
                }
            }
        }


        TEST(RoundTrip, Trivial)
        {
            const unsigned size1 = 3;
            const unsigned size2 = 5;

            Array2DFixed<unsigned, size1, size2> a;

            unsigned counter = 0;
            for (unsigned x = 0; x < size1; ++x)
            {
                for (unsigned y = 0; y < size2; ++y)
                {
                    a.At(x, y) =  counter++;
                }
            }

            std::stringstream stream;
            a.Write(stream);

            Array2DFixed<unsigned, size1, size2> b(stream);

            counter = 0;
            for (unsigned x = 0; x < size1; ++x)
            {
                for (unsigned y = 0; y < size2; ++y)
                {
                    EXPECT_EQ(b.At(x, y), counter++);
                }
            }
        }
    }
}
