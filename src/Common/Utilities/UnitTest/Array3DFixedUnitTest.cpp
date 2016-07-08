#include <sstream>

#include "gtest/gtest.h"

#include "Array.h"
#include "ConstructorDestructorCounter.h"


namespace BitFunnel
{
    namespace Array3DFixedUnitTest
    {
        TEST(Initialization, Trivial)
        {
            ConstructorDestructorCounter::ClearCount();

            const unsigned size1 = 4;
            const unsigned size2 = 17;
            const unsigned size3 = 23;

            {
                Array3DFixed<ConstructorDestructorCounter, size1, size2, size3> a;

                EXPECT_EQ(ConstructorDestructorCounter::GetConstructorCount(), size1 * size2 * size3);
                EXPECT_EQ(ConstructorDestructorCounter::GetDestructorCount(), 0u);
            }

            EXPECT_EQ(ConstructorDestructorCounter::GetDestructorCount(), size1 * size2 * size3);
        }


        TEST(FieldAccess, Trivial)
        {
            const unsigned size1 = 3;
            const unsigned size2 = 5;
            const unsigned size3 = 7;

            Array3DFixed<unsigned, size1, size2, size3> a;

            unsigned counter = 0;
            for (unsigned x = 0; x < size1; ++x)
            {
                for (unsigned y = 0; y < size2; ++y)
                {
                    for (unsigned z = 0; z < size3; ++z)
                    {
                        a.At(x, y, z) =  counter++;
                    }
                }
            }

            counter = 0;
            for (unsigned x = 0; x < size1; ++x)
            {
                for (unsigned y = 0; y < size2; ++y)
                {
                    for (unsigned z = 0; z < size3; ++z)
                    {
                        EXPECT_EQ(a.At(x, y, z), counter++);
                    }
                }
            }
        }


        TEST(RoundTrip, Trivial)
        {
            const unsigned size1 = 3;
            const unsigned size2 = 5;
            const unsigned size3 = 7;

            Array3DFixed<unsigned, size1, size2, size3> a;

            unsigned counter = 0;
            for (unsigned x = 0; x < size1; ++x)
            {
                for (unsigned y = 0; y < size2; ++y)
                {
                    for (unsigned z = 0; z < size3; ++z)
                    {
                        a.At(x, y, z) =  counter++;
                    }
                }
            }

            std::stringstream stream;
            a.Write(stream);

            Array3DFixed<unsigned, size1, size2, size3> b(stream);

            counter = 0;
            for (unsigned x = 0; x < size1; ++x)
            {
                for (unsigned y = 0; y < size2; ++y)
                {
                    for (unsigned z = 0; z < size3; ++z)
                    {
                        EXPECT_EQ(b.At(x, y, z), counter++);
                    }
                }
            }
        }
    }
}
