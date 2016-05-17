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

#pragma once

namespace CmdLine
{
    //*************************************************************************
    //
    // IValidator<T>
    //
    // Abstract base class for parameter value validators.
    //
    //*************************************************************************
    template <class T>
    class IValidator
    {
    public:
        virtual ~IValidator() {};
        virtual bool IsValid(const T& value) const = 0;
        virtual void Description(std::ostream& out) const = 0;
    };

    //*************************************************************************
    //
    // ComparisonValidator<T>
    //
    // Validates a parameter value against a constant using <, <=, >, >=, ==,
    // and !=.
    //*************************************************************************
    template <class T>
    class ComparisonValidator : public IValidator<T>
    {
    public:
        enum ComparisonTypeEnum {LessThan, LessThanOrEqual, GreaterThan, GreaterThanOrEqual, Equal, NotEqual};

        ComparisonValidator(ComparisonTypeEnum t, const T& value);
        bool IsValid(const T& value) const;
        void Description(std::ostream& out) const;

    private:
        ComparisonTypeEnum m_type;
        T m_value;
    };

#define DECLARE_COMPARISON_VALIDATOR(TYPE)\
    template <class T>\
    std::auto_ptr<IValidator<T>> ##TYPE##(const T& value);

    DECLARE_COMPARISON_VALIDATOR(LessThan);
    DECLARE_COMPARISON_VALIDATOR(LessThanOrEqual);
    DECLARE_COMPARISON_VALIDATOR(GreaterThan);
    DECLARE_COMPARISON_VALIDATOR(GreaterThanOrEqual);
    DECLARE_COMPARISON_VALIDATOR(Equal);
    DECLARE_COMPARISON_VALIDATOR(NotEqual);

#undef DECLARE_COMPARISON_VALIDATOR

    //*************************************************************************
    //
    // PairValidator<T>
    //
    // Represents a pair of validators that must both be satisfied. 
    // Created to validate parameter values lie within a range.
    //*************************************************************************
    template <class T>
    class PairValidator : public IValidator<T>
    {
    public:
        PairValidator(std::auto_ptr<IValidator<T>> first, std::auto_ptr<IValidator<T>> second);

        bool IsValid(const T& value) const;
        void Description(std::ostream& out) const;

    private:
        std::auto_ptr<IValidator<T>> m_first;
        std::auto_ptr<IValidator<T>> m_second;
    };

    template <class T>
    std::auto_ptr<IValidator<T>> Range(std::auto_ptr<IValidator<T>> first, std::auto_ptr<IValidator<T>> second);

    //*************************************************************************
    //
    // SetValidator<T>
    //
    // Validates that a parameter value is a member of a set of values.
    //*************************************************************************
    template <class T>
    class SetValidator : public IValidator<T>
    {
    public:
        SetValidator();

        void RegisterField(const T& value);

        bool IsValid(const T& value) const;
        void Description(std::ostream& out) const;

    private:
        std::vector<T> m_values;
    };

    template <class T>
    std::auto_ptr<IValidator<T>> From(const T& a);

    template <class T>
    std::auto_ptr<IValidator<T>> From(const T& a, const T& b);

    template <class T>
    std::auto_ptr<IValidator<T>> From(const T& a, const T& b, const T& c);

    template <class T>
    std::auto_ptr<IValidator<T>> From(const T& a, const T& b, const T& c, const T& d);

    template <class T>
    std::auto_ptr<IValidator<T>> From(const T& a, const T& b, const T& c, const T& d, const T& e);

    template <class T>
    std::auto_ptr<IValidator<T>> From(const std::vector<T>& values);


    //*************************************************************************
    //
    // Template definitions after this point
    //
    //*************************************************************************


    //*************************************************************************
    //
    // ComparisionValidator<T> validator.
    //
    //*************************************************************************
    template <class T>
    ComparisonValidator<T>::ComparisonValidator(ComparisonTypeEnum t, const T& value)
        : m_type(t),
          m_value(value)
    {
    }

    template <class T>
    bool ComparisonValidator<T>::IsValid(const T& value) const
    {
        switch (m_type)
        {
        case LessThan:
            return value < m_value;
            break;
        case LessThanOrEqual:
            return value <= m_value;
            break;
        case GreaterThan:
            return value > m_value;
            break;
        case GreaterThanOrEqual:
            return value >= m_value;
            break;
        case Equal:
            return value == m_value;
            break;
        case NotEqual:
            return value != m_value;
            break;
        };
        return false;
    }

    template <class T>
    void ComparisonValidator<T>::Description(std::ostream& out) const
    {
        switch (m_type)
        {
        case LessThan:
            out << "must be less than ";
            break;
        case LessThanOrEqual:
            out << "must be less than or equal to ";
            break;
        case GreaterThan:
            out << "must be greater than ";
            break;
        case GreaterThanOrEqual:
            out << "must be greater than or equal to ";
            break;
        case Equal:
            out << "must be equal ";
            break;
        case NotEqual:
            out << "cannot equal ";
            break;
        };
        out << m_value;
    }

#define DEFINE_COMPARISON_VALIDATOR(TYPE)\
    template <class T>\
    std::auto_ptr<IValidator<T>> TYPE##(const T& value)\
    {\
    return std::auto_ptr<IValidator<T>>(new ComparisonValidator<T>(ComparisonValidator<T>::##TYPE##, value));\
    }

    DEFINE_COMPARISON_VALIDATOR(LessThan);
    DEFINE_COMPARISON_VALIDATOR(LessThanOrEqual);
    DEFINE_COMPARISON_VALIDATOR(GreaterThan);
    DEFINE_COMPARISON_VALIDATOR(GreaterThanOrEqual);
    DEFINE_COMPARISON_VALIDATOR(Equal);
    DEFINE_COMPARISON_VALIDATOR(NotEqual);

#undef DEFINE_COMPARISION_VALIDATOR

    //*************************************************************************
    //
    // PairValidator<T>
    //
    //*************************************************************************
    template <class T>
    PairValidator<T>::PairValidator(std::auto_ptr<IValidator<T>> first, std::auto_ptr<IValidator<T>> second)
        : m_first(first),
          m_second(second)
    {
    }

    template <class T>
    bool PairValidator<T>::IsValid(const T& value) const
    {
        return m_first.get()->IsValid(value) && m_second.get()->IsValid(value);
    }

    template <class T>
    void PairValidator<T>::Description(std::ostream& out) const
    {
        m_first.get()->Description(out);
        out << " and ";
        m_second.get()->Description(out);
    }

    template <class T>
    std::auto_ptr<IValidator<T>> Range(std::auto_ptr<IValidator<T>> first, std::auto_ptr<IValidator<T>> second)
    {
        return std::auto_ptr<IValidator<T>>(new PairValidator<T>(first, second));
    }

    //*************************************************************************
    //
    // SetValidator<T>
    //
    //*************************************************************************
    template <class T>
    SetValidator<T>::SetValidator()
    {
    }

    template <class T>
    void SetValidator<T>::RegisterField(const T& value)
    {
        m_values.push_back(value);
    }

    template <class T>
    bool SetValidator<T>::IsValid(const T& value) const
    {
        bool success = false;
        for (unsigned i = 0 ; i < m_values.size(); ++i)
        {
            if (value == m_values[i])
            {
                success = true;
                break;
            }
        }
        return success;
    }

    template <class T>
    void SetValidator<T>::Description(std::ostream& out) const
    {
        out << "legal values are ";
        for (unsigned i = 0 ; i < m_values.size(); ++i)
        {
            if (i > 0)
            {
                if (i == m_values.size() - 1)
                {
                    out << ", and ";
                }
                else
                {
                    out << ", ";
                }
            }
            out << m_values[i];
        }
    }

    template <class T>
    std::auto_ptr<IValidator<T>> From(const T& a)
    {
        SetValidator<T>* validator = new SetValidator<T>();
        validator->RegisterField(a);
        return std::auto_ptr<IValidator<T>>(validator);
    }

    template <class T>
    std::auto_ptr<IValidator<T>> From(const T& a, const T& b)
    {
        SetValidator<T>* validator = new SetValidator<T>();
        validator->RegisterField(a);
        validator->RegisterField(b);
        return std::auto_ptr<IValidator<T>>(validator);
    }

    template <class T>
    std::auto_ptr<IValidator<T>> From(const T& a, const T& b, const T& c)
    {
        SetValidator<T>* validator = new SetValidator<T>();
        validator->RegisterField(a);
        validator->RegisterField(b);
        validator->RegisterField(c);
        return std::auto_ptr<IValidator<T>>(validator);
    }

    template <class T>
    std::auto_ptr<IValidator<T>> From(const T& a, const T& b, const T& c, const T& d)
    {
        SetValidator<T>* validator = new SetValidator<T>();
        validator->RegisterField(a);
        validator->RegisterField(b);
        validator->RegisterField(c);
        validator->RegisterField(d);
        return std::auto_ptr<IValidator<T>>(validator);
    }

    template <class T>
    std::auto_ptr<IValidator<T>> From(const T& a, const T& b, const T& c, const T& d, const T& e)
    {
        SetValidator<T>* validator = new SetValidator<T>();
        validator->RegisterField(a);
        validator->RegisterField(b);
        validator->RegisterField(c);
        validator->RegisterField(d);
        validator->RegisterField(e);
        return std::auto_ptr<IValidator<T>>(validator);
    }

    template <class T>
    std::auto_ptr<IValidator<T>> From(const std::vector<T>& values)
    {
        SetValidator<T>* validator = new SetValidator<T>();
        for (unsigned i = 0 ; i < values.size(); ++i)
        {
            validator->RegisterField(values[i]);
        }
        return std::auto_ptr<IValidator<T>>(validator);
    }
}
