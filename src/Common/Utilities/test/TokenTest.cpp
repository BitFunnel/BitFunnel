// The MIT License (MIT)

// Copyright (c) 2016, Microsoft

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#include "gtest/gtest.h"

#include "BitFunnel/Token.h"

namespace BitFunnel
{
    namespace TokenTest
    {
        static const unsigned c_maxTokenCount = 1000;

        // A test implementation of ITokenListener which allows to check if a
        // token with particular serial number has been returned. For every
        // serial number it keeps a counter which is incremented when a token
        // with this number is issued, and decremented when it is returned.
        // At the end we can verify that all of the counters have a zero value
        // which means that all of the tokens were returned properly.
        class TestTokenListener : public ITokenListener
        {
        public:
            TestTokenListener();

            // Record that a token with the given serial number is in flight.
            void RecordIssuedToken(SerialNumber serialNumber);

            // Returns true if a token with the given serial number is
            // in flight.
            bool IsInFlight(SerialNumber serialNumber) const;

            // Verifies that all issued tokens have been returned properly.
            void VerifyAllTokensReturned() const;

            //
            // ITokenListener API.
            //
            virtual void OnTokenComplete(SerialNumber serialNumber);

        private:
            // An array of tokens counters. A value of 1 means the token was
            // issued and not returned. A value of 0 means the token was either
            // not issued, or issued and returned. A negative value means the
            // token was returned multiple times. At the end of the test, all
            // items in the array should have a value of 0 to make a test pass.
            int m_tokensCounter[c_maxTokenCount];
        };


        TestTokenListener::TestTokenListener()
        {
            for (unsigned i = 0; i < c_maxTokenCount; ++i)
            {
                m_tokensCounter[i] = 0;
            }
        }


        void TestTokenListener::RecordIssuedToken(SerialNumber serialNumber)
        {
            ASSERT_EQ(m_tokensCounter[serialNumber], 0);

            m_tokensCounter[serialNumber] = 1;
        }


        bool TestTokenListener::IsInFlight(SerialNumber serialNumber) const
        {
            return m_tokensCounter[serialNumber] == 1;
        }


        void TestTokenListener::VerifyAllTokensReturned() const
        {
            for (unsigned i = 0; i < c_maxTokenCount; ++i)
            {
                ASSERT_EQ(m_tokensCounter[i], 0);
            }
        }


        void TestTokenListener::OnTokenComplete(SerialNumber serialNumber)
        {
            const int tokenCounterValue = --m_tokensCounter[serialNumber];
            ASSERT_TRUE(tokenCounterValue >= 0);
        }


        // Test which covers creation of tokens and verifies that tokens
        // callback their issuer at the end of their lifetime.
        TEST(Token, Basic)
        {
            TestTokenListener issuer;

            issuer.VerifyAllTokensReturned();

            SerialNumber lastSerialNumber = 1;

            {
                ASSERT_TRUE(!issuer.IsInFlight(lastSerialNumber));

                Token token(issuer, lastSerialNumber);
                issuer.RecordIssuedToken(lastSerialNumber);

                ASSERT_EQ(token.GetSerialNumber(), lastSerialNumber);
                ASSERT_TRUE(issuer.IsInFlight(lastSerialNumber));

                lastSerialNumber++;
            }

            // A token should be returned by now.
            ASSERT_TRUE(!issuer.IsInFlight(lastSerialNumber));
            issuer.VerifyAllTokensReturned();

            {
                Token token1(issuer, lastSerialNumber);
                issuer.RecordIssuedToken(lastSerialNumber);
                ASSERT_EQ(token1.GetSerialNumber(), lastSerialNumber);

                lastSerialNumber++;

                {
                    Token token2(issuer, lastSerialNumber);
                    issuer.RecordIssuedToken(lastSerialNumber);
                    ASSERT_EQ(token2.GetSerialNumber(), lastSerialNumber);

                    // Both tokens should be in flight.
                    ASSERT_TRUE(issuer.IsInFlight(lastSerialNumber - 1));
                    ASSERT_TRUE(issuer.IsInFlight(lastSerialNumber));
                }

                // token2 has been returned.
                ASSERT_TRUE(issuer.IsInFlight(lastSerialNumber - 1));
                ASSERT_TRUE(!issuer.IsInFlight(lastSerialNumber));

                lastSerialNumber++;
            }

            issuer.VerifyAllTokensReturned();
        }


        // Generate a token and return it by value. A token with this
        // serial number should only be returned once - when it goes out of
        // scope in the calling context.
        /*
          // TODO: understand this old code. This seems like it can't work because
          // Token has an implictly deleted copy constructor
        const Token GenerateToken(TestTokenListener& issuer, SerialNumber serialNumber)
        {
            issuer.RecordIssuedToken(serialNumber);
            return Token(issuer, serialNumber);
        }
        */

        // Test which verifies that copy is done by moving a token object.
        TEST(Token, Copy)
        {
            TestTokenListener issuer;

            issuer.VerifyAllTokensReturned();

            SerialNumber lastSerialNumber = 1;

            {
                ASSERT_TRUE(!issuer.IsInFlight(lastSerialNumber));

                // TODO: see TODO for this function.
                // Token token = GenerateToken(issuer, lastSerialNumber);
                issuer.RecordIssuedToken(lastSerialNumber);
                Token token(issuer, lastSerialNumber);

                // Even though token was returned by value, it should not be
                // marked as returned in the issuer yet since it is still valid
                // in this scope.
                ASSERT_TRUE(issuer.IsInFlight(lastSerialNumber));

                // Try to move a token to a different object. It should still
                // notify the issuer with this serial number only once.
                Token tokenCopy(std::move(token));
                ASSERT_TRUE(issuer.IsInFlight(lastSerialNumber));
            }

            // Token goes out of scope and should be marked as returned.
            ASSERT_TRUE(!issuer.IsInFlight(lastSerialNumber));

            issuer.VerifyAllTokensReturned();
        }
    }
}
