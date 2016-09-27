// Some portions of this file were adapted from GTest. The GTest license
// appears below.

// Copyright 2007, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include <ostream>
#include <sstream>

// TODO: File name and line numbers.
// TODO: Differentiate between strings and pointers.
// TODO: String compare.

namespace Logging
{
    template <typename T>
    void StreamOut(std::stringstream& stream,
                   typename std::enable_if<!std::is_same<T, bool>::value &&
                                           !std::is_pointer<T>::value &&
                                           !std::is_convertible<T, char const *>::value, T const &>::type value)
    {
        stream << value;
    }

    template <typename T>
    void StreamOut(std::stringstream& stream,
                   typename std::enable_if<std::is_pointer<T>::value && !std::is_convertible<T, char const *>::value, T const &>::type value)
    {
        if (value == nullptr)
        {
            stream << "(nullptr)";
        }
        else
        {
            stream << std::hex << static_cast<void const *>(value);
        }
    }

    template <typename T>
    void StreamOut(std::stringstream& stream,
                   typename std::enable_if<std::is_convertible<T, char const *>::value, T const &>::type value)
    {
        if (value == nullptr)
        {
            stream << "(empty string)";
        }
        else
        {
            stream << value;
        }
    }

    template <typename T>
    void StreamOut(std::stringstream& stream,
                   typename std::enable_if<std::is_same<T, bool>::value, T const &>::type value)
    {
        stream << (value ? "true" : "false");
    }


    class CheckException
    {
    public:
        CheckException()
        {
        }

        CheckException(CheckException const & other)
        {
            m_message << other.m_message.str();
        }

        // Streams a custom failure message into this object.
        template <typename T>
        CheckException& operator<<(T const & value)
        {
            StreamOut<T>(m_message, value);
            return *this;
        }

        // Allows streaming basic output manipulators such as endl or flush
        // into this object.
        CheckException& operator<<(
            ::std::ostream& (*basic_manipulator)(::std::ostream& stream))
        {
            m_message << basic_manipulator;
            return *this;
        }

        std::string GetMessage() const
        {
            return m_message.str();
        }

    private:
        std::stringstream m_message;
    };


template <typename T1, typename T2>
class CheckResult
{
public:
    //
    // CheckResult holds information about an assertion check, allowing
    // success detection and exception generation.
    //
    // IMPORTANT: CheckResult constructor must do no real work so that the
    // optimizing compiler will generate no code for the constructor. This
    // ensures that the cost of a successful assert is only the cost of the
    // if-statement.
    //
    CheckResult(T1 const& val1,
                T2 const& val2,
                bool succeeded)
      : m_val1(val1),
        m_val2(val2),
        m_succeeded(succeeded)
    {
    }

    CheckException Exception(char const* expr1,
                             char const* expr2,
                             char const* op) const
    {
        return CheckException()
            << "Expected: (" << expr1 << ") " << op
            << " (" << expr2 << "), actual: (" << m_val1
            << ") vs (" << m_val2 << ")" << std::endl;
    }

    operator bool() const
    {
        return m_succeeded;
    }

private:
    T1 const & m_val1;
    T2 const & m_val2;
    const bool m_succeeded;
};


class CheckBooleanResult
{
public:
    //
    // CheckResult holds information about an assertion check, allowing
    // success detection and exception generation.
    //
    // IMPORTANT: CheckResult constructor must do no real work so that the
    // optimizing compiler will generate no code for the constructor. This
    // ensures that the cost of a successful assert is only the cost of the
    // if-statement.
    //
    CheckBooleanResult(bool value,
                       bool expected)
      : m_value(value),
        m_expected(expected)
    {
    }

    CheckException Exception(char const* expressionText) const
    {
        return CheckException()
            << "Value of: " << expressionText << std::endl
            << "  Actual: " << m_value << std::endl
            << "  Expected: " << m_expected << std::endl;
    }

    operator bool() const
    {
        return m_value == m_expected;
    }

private:
    const bool m_value;
    const bool m_expected;
};


//
// CmpHelperOP returns a CheckResult configured with the result
// of the compare operation and references to val1 and val2.
//
// IMPORTANT: CmpHelperOP is implemented as a function to ensure
// that val1 and val2 are evaluated exactly once.
//
#define BFTEST_IMPL_CMP_HELPER(op_name, op)                \
template <typename T1, typename T2>                        \
CheckResult<T1,T2> CmpHelper##op_name(T1 const & val1,   \
                                      T2 const & val2)   \
{                                                          \
    return CheckResult<T1,T2>(val1, val2, val1 op val2);   \
}

BFTEST_IMPL_CMP_HELPER(EQ, == );
BFTEST_IMPL_CMP_HELPER(NE, != );
BFTEST_IMPL_CMP_HELPER(LE, <= );
BFTEST_IMPL_CMP_HELPER(LT, < );
BFTEST_IMPL_CMP_HELPER(GE, >= );
BFTEST_IMPL_CMP_HELPER(GT, > );

#undef BFTEST_IMPL_CMP_HELPER

//
// CHECK_OP performs the assert checking.
//
// IMPORTANT: auto variable op must be scoped to the
// if-statement to allow multiple CHECK_OP macros in
// the same scope.
//
// IMPORTANT: the throw in the else clause should
// not end with a semicolon so that is is possible for
// users of the macro to shift values into the Exception
// before it is thrown, e.g.
//    CHECK_EQ(x,y) << "Values must be equal.";
//
#define CHECK_OP(val1, val2, op_name)                \
if (const auto op =                                  \
    ::Logging::CmpHelper##op_name(val1, val2))       \
{                                                    \
}                                                    \
else                                                 \
    throw op.Exception(#val1, #val2, #op_name)

#define CHECK_NE(val1, val2) \
    CHECK_OP(val1, val2, NE)
#define CHECK_EQ(val1, val2) \
    CHECK_OP(val1, val2, EQ)
#define CHECK_LE(val1, val2) \
    CHECK_OP(val1, val2, LE)
#define CHECK_LT(val1, val2) \
    CHECK_OP(val1, val2, LT)
#define CHECK_GE(val1, val2) \
    CHECK_OP(val1, val2, GE)
#define CHECK_GT(val1, val2) \
    CHECK_OP(val1, val2, GT)

#define CHECK_TRUE(val)                                       \
if (const auto op = ::Logging::CheckBooleanResult(val, true)) \
{                                                             \
}                                                             \
else                                                          \
    throw op.Exception(#val)

#define CHECK_FALSE(val)                                       \
if (const auto op = ::Logging::CheckBooleanResult(val, false)) \
{                                                              \
}                                                              \
else                                                           \
    throw op.Exception(#val)


}   // namespace Logging
