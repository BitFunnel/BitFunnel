#pragma once

#include <ostream>
#include <sstream>

namespace Logging
{
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
        template <typename T> CheckException& operator<<(const T& value)
        {
            m_message << value;
            return *this;
        }

        // Allows streaming basic output manipulators such as endl or flush into
        // this object.
        CheckException& operator<<(
            ::std::ostream& (*basic_manipulator)(::std::ostream& stream))
        {
            m_message << basic_manipulator;
            return *this;
        }

    private:
        std::stringstream m_message;
    };


template <typename T1, typename T2>
class CmpHelperBase
{
public:
    CmpHelperBase(T1 const& val1,
                  T2 const& val2,
                  bool succeeded)
      : m_val1(val1),
        m_val2(val2),
        m_succeeded(succeeded)
    {
    }

    CheckException Exception(char const* expr1,
                             char const* expr2,
                             char const* op)
    {
        return CheckException()
            << "Expected: (" << expr1 << ") " << op
            << " (" << expr2 << "), actual: " << m_val1
            << " vs " << m_val2;
    }

    bool Succeeded() const
    {
        return m_succeeded;
    }

private:
    T1 const & m_val1;
    T2 const & m_val2;
    const bool m_succeeded;
};


#define BFTEST_IMPL_CMP_HELPER(op_name, op)                \
template <typename T1, typename T2>                        \
CmpHelperBase<T1,T2> CmpHelper##op_name(T1 const & val1,   \
                                        T2 const & val2)   \
{                                                          \
    return CmpHelperBase<T1,T2>(val1, val2, val1 op val2); \
}

BFTEST_IMPL_CMP_HELPER(EQ, != );
BFTEST_IMPL_CMP_HELPER(NE, != );

#undef BFTEST_IMPL_CMP_HELPER

#define CHECK_OP(val1, val2, op_name)                \
auto op = ::Logging::CmpHelper##op_name(val1, val2); \
if (op.Succeeded()) {                                \
}                                                    \
else                                                 \
    throw op.Exception(#val1, #val2, #op_name)

#define CHECK_NE(val1, val2) \
    CHECK_OP(val1, val2, NE)
#define CHECK_EQ(val1, val2) \
    CHECK_OP(val1, val2, EQ)
}
