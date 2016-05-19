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

#include <ostream>
#include <string>
#include <vector>

#include "Constraint.h"  // Template definitions call members of IConstraint.
#include "Validator.h"   // Template definitions call members of IValidator.

// C4505 must be enabled for the entire header file and everything that comes 
// after because the compiler generates C4505 after parsing all files.
#ifdef BITFUNNEL_PLATFORM_WINDOWS
#pragma warning(disable:4505)
#endif // BITFUNNEL_PLATFORM_WINDOWS

// C4250 is caused by a compiler bug - see http://connect.microsoft.com/VisualStudio/feedback/details/101259/disable-warning-c4250-class1-inherits-class2-member-via-dominance-when-weak-member-is-a-pure-virtual-function
#ifdef BITFUNNEL_PLATFORM_WINDOWS
#pragma warning(disable:4250)
#endif // BITFUNNEL_PLATFORM_WINDOWS


namespace CmdLine
{
    //*************************************************************************
    //
    // IParameter
    //
    // Abstract base class for parameters. Not specialized to any particular
    // parameter value type.
    //
    //*************************************************************************
    class IParameter
    {
    public:
        virtual ~IParameter() {};

        // Returns the name of the parameter. The name is used for documentation
        // and optional parameter flags.
        virtual const std::string& GetName() const = 0;

        // Used for laying out the help message. Returns width required for a 
        // column that contains the name of this parameter and all of its sub
        // parameters (e.g. for an optional parameter list).
        virtual unsigned GetWidth() const = 0;

        // True if this parameter has been successfully parsed from the command
        // line.
        virtual bool IsActivated() const = 0;

        // Attempt to parse this parameter, starting at argv[currentArg].
        // If successful, currentArg will point to the position after this parameter
        // in the command line and the return value will be true. Otherwise the
        // return value will be false.
        virtual bool TryParse(std::ostream& error, 
                              unsigned& currentArg, 
                              unsigned argc, char const* const* argv) = 0;

        // Prints the usage message for this parameter to out. The name of this 
        // parameter and its sub-parameters will appear in a first column with
        // width == left. The descriptions will appear as blocks of text in a second
        // column that targets width == targetWidth (the actual formatted width may
        // exceed targetWidth because of long words).
        virtual void Usage(std::ostream& out, unsigned left, unsigned targetWidth) const = 0;
        virtual void Syntax(std::ostream& out) const = 0;
        virtual void Description(std::ostream& out) const = 0;
    };

    //*************************************************************************
    //
    // IRequiredParameter
    //
    // Abstract base class for required parameters. Not specialized to any 
    // particular parameter value type.
    //
    //*************************************************************************
    class IRequiredParameter : public virtual IParameter
    {
    public:
        virtual ~IRequiredParameter() {};
    };

    //*************************************************************************
    //
    // IOptionalParameter
    //
    // Abstract base class for optional parameters. Not specialized to any 
    // particular parameter value type.
    //
    //*************************************************************************
    class IOptionalParameter : public virtual IParameter
    {
    public:
        virtual ~IOptionalParameter() {};

        virtual bool MatchesFlag(const char* name) const = 0;
        virtual bool HasValue() const = 0;
        virtual bool HasDefaultValue() const = 0;
    };

    //*************************************************************************
    //
    // ParameterBase
    //
    // Base class that supplies portions of IParameter and convenience
    // functions not related to a specific value type. ParameterBase is
    // specialize by templated subclasses that add functions related to
    // the value's type.
    //*************************************************************************
    class ParameterBase : public virtual IParameter
    {
    public:
        ParameterBase(const char* name, const char* description);

        const std::string& GetName() const;
        unsigned GetWidth() const;
        bool IsActivated() const;
        void Usage(std::ostream& out, unsigned left, unsigned targetWidth) const;
        void Description(std::ostream& out) const;

        static void Format(std::ostream& out, int value);
        static void Format(std::ostream& out, double value);
        static void Format(std::ostream& out, bool value);
        static void Format(std::ostream& out, const char* value);

    protected:
        template <class T> const char* GetTypeName() const;

        static bool TryParse(const char* input, int& value);
        static bool TryParse(const char* input, double& value);
        static bool TryParse(const char* input, bool& value);
        static bool TryParse(const char* input, const char*& value);

        bool m_isActivated;

    private:
        std::string m_name;
        std::string m_description;
    };

    //*************************************************************************
    //
    // RequiredParameter<T>
    //
    // Represents a required (non-optional) command line parameter of type T 
    // with an optional validator for the value. Note that T is restricted to 
    // int, bool, double, or const char*.
    //
    // Boolean values on the command line are represented by the strings,
    // "true" and "false".
    //
    //*************************************************************************
    template <class T>
    class RequiredParameter : public IRequiredParameter, public ParameterBase
    {
    public:
        RequiredParameter(const char* name, const char* description, 
                          std::auto_ptr<IValidator<T>> validator = std::auto_ptr<IValidator<T>>(NULL));
        ~RequiredParameter();

        // Local methods
        T GetValue() const;
        operator T() const;     // Same as GetValue();

        // IParameter methods
        bool TryParse(std::ostream& error, unsigned& currentArg, unsigned argc, char const* const* argv);
        void Syntax(std::ostream& out) const;
        void Description(std::ostream& out) const;

    private:
        T m_value;
        std::auto_ptr<IValidator<T>> m_validator;
    };

    //*************************************************************************
    //
    // OptionalParameter<T>
    //
    // Represents an optional command line parameter of type T. On the command
    // line, optional parameters start with a flag which consists of a '-' or 
    // '/' followed by the name of the parameter. The value of the parameter
    // follows the flag. (e.g. -width 5).
    //
    // As with RequiredParameter<T>, type T is restricted to int, bool, double, 
    // or const char*.
    //
    // Boolean values on the command line are represented by the strings,
    // "true" and "false".
    //
    // While OptionalParameter<bool> is supported, OptionalParameterList<bool>
    // is recommended in order to provide better command line syntax:
    //   OptionalParameterList<bool>: "-doSomething"
    //       OptionalParameter<bool>: "-doSomething true"
    //*************************************************************************
    template <class T>
    class OptionalParameter : public IOptionalParameter, public ParameterBase
    {
    public:
        OptionalParameter(const char* name,
                          const char* description,
                          std::auto_ptr<IValidator<T>> validator = std::auto_ptr<IValidator<T>>(NULL));
        OptionalParameter(const char* name,
                          const char* description,
                          const T& defaultValue,
                          std::auto_ptr<IValidator<T>> validator = std::auto_ptr<IValidator<T>>(NULL));

        // Local methods
        T GetValue() const;
        operator T() const;     // Same as GetValue().

        T GetDefaultValue() const;

        virtual std::auto_ptr<IConstraint> Requires(const IOptionalParameter& p) const;

        // IParameter methods
        bool TryParse(std::ostream& error, unsigned& currentArg, unsigned argc, char const* const* argv);
        void Syntax(std::ostream& out) const;
        void Description(std::ostream& out) const;

        // IOptionalParameter methods
        bool MatchesFlag(const char* name) const;
        bool HasValue() const;
        bool HasDefaultValue() const;

    protected:
        void SetValue(const T& value);

    private:
        T m_value;

        bool m_hasDefaultValue;
        T m_defaultValue;

        std::auto_ptr<IValidator<T>> m_validator;
    };

    //*************************************************************************
    //
    // OptionalParameterList
    //
    // Represents an optional list of parameters. On the command
    // line, an optional parameters list starts with a flag which consists of 
    // a '-' or '/' followed by the name of the parameter. A list of parameter
    // values follow the flag (e.g. -dimensions 5 10).
    //
    // The parameter values are defined with the AddParameter() method.
    //
    //*************************************************************************
    class OptionalParameterList : public OptionalParameter<bool>
    {
    public:
        OptionalParameterList(const char* name,
                              const char* description);

        // Local methods
        void AddParameter(IRequiredParameter& parameter);

        // IParameter methods
        unsigned GetWidth() const;
        bool TryParse(std::ostream& error, unsigned& currentArg, unsigned argc, char const* const* argv);

        void Usage(std::ostream& out, unsigned left, unsigned targetWidth) const;
        void Syntax(std::ostream& out) const;
//        void Description(std::ostream& out) const;

    private:
        std::vector<IRequiredParameter*> m_parameters;
    };


    //*************************************************************************
    //
    // Template definitions after this point
    //
    //*************************************************************************

    //*************************************************************************
    //
    // RequiredParameter<T>
    //
    //*************************************************************************
    template <class T>
    RequiredParameter<T>::RequiredParameter(const char* name,
                                            const char* description,
                                            std::auto_ptr<IValidator<T>> validator)
        : ParameterBase(name, description),
          m_validator(validator)
    {
    }

    template <class T>
    RequiredParameter<T>::~RequiredParameter()
    {
    }

    template <class T>
    T RequiredParameter<T>::GetValue() const
    {
        return m_value;
    }

    template <class T>
    RequiredParameter<T>::operator T() const
    {
        return GetValue();
    }

    template <class T>
    bool RequiredParameter<T>::TryParse(std::ostream& error, unsigned& currentArg, 
                                        unsigned argc, char const* const* argv)
    {
        bool success = false;
        if (currentArg < argc && ParameterBase::TryParse(argv[currentArg], m_value))
        {
            ++currentArg;

            IValidator<T>* validator = m_validator.get();
            if (validator != NULL)
            {
                success = validator->IsValid(m_value);
                if (!success)
                {
                    error << "Validation error: ";
                    error << GetName() << " ";
                    validator->Description(error);
                    error << "." << std::endl;
                }
            }
            else
            {
                success = true;
            }
        }
        else
        {
            error << "Expected " << GetTypeName<T>() << " parameter ";
            Syntax(error);
            error << "." << std::endl;
        }

        m_isActivated = success;
        return success;
    }

    template <class T>
    void RequiredParameter<T>::Syntax(std::ostream& out) const
    {
        out << '<';
        out << GetName();
        out << '>';
    }

    template <class T>
    void RequiredParameter<T>::Description(std::ostream& out) const
    {
        ParameterBase::Description(out);
        out << " (" << GetTypeName<T>();
        if (m_validator.get() != NULL)
        {
            out << ", ";
            m_validator.get()->Description(out);
        }
        out << ")";
    }

    template <class T>
    std::ostream& operator<<(std::ostream& out, const RequiredParameter<T>& value)
    {
        ParameterBase::Format(out, value.GetValue());
        return out;
    }

    //*************************************************************************
    //
    // OptionalParameter<T>
    //
    //*************************************************************************
    template <class T>
    OptionalParameter<T>::OptionalParameter(const char* name,
                                            const char* description,
                                            const T& defaultValue, 
                                            std::auto_ptr<IValidator<T>> validator)
        : ParameterBase(name, description),
          m_hasDefaultValue(true),
          m_defaultValue(defaultValue),
          m_value(defaultValue),
          m_validator(validator)
    {
    }

    template <class T>
    OptionalParameter<T>::OptionalParameter(const char* name,
                                            const char* description,
                                            std::auto_ptr<IValidator<T>> validator)
        : ParameterBase(name, description),
          m_hasDefaultValue(false),
          m_validator(validator)
    {
    }

    template <class T>
    T OptionalParameter<T>::GetValue() const
    {
        return m_value;
    }

    template <class T>
    OptionalParameter<T>::operator T() const
    {
        return GetValue();
    }

    template <class T>
    bool OptionalParameter<T>::HasDefaultValue() const
    {
        return m_hasDefaultValue;
    }

    template <class T>
    void OptionalParameter<T>::SetValue(const T& value)
    {
        m_value = value;
    }

    template <class T>
    bool OptionalParameter<T>::HasValue() const
    {
        return m_hasDefaultValue || m_isActivated;
    }

    template <class T>
    T OptionalParameter<T>::GetDefaultValue() const
    {
        return m_defaultValue;
    }

    template <class T>
    std::auto_ptr<IConstraint> OptionalParameter<T>::Requires(const IOptionalParameter& p) const
    {
        return std::auto_ptr<IConstraint>(new CoexistenceConstraint(CoexistenceConstraint::Implies, *this, p));
    }

    template <class T>
    bool OptionalParameter<T>::MatchesFlag(const char* name) const
    {
        bool success = false;

        if (name[0] == '-' || name[0] == '/')
        {
            if (GetName().compare(name + 1) == 0)
            {
                success = true;
            }
        }

        return success;
    }

    template <class T> 
    bool OptionalParameter<T>::TryParse(std::ostream& error, unsigned& currentArg, unsigned argc, char const* const* argv)
    {
        bool success = false;
        
        // Skip over the parameter flag.
        ++currentArg;

        if (currentArg >= argc)
        {
            success = false;
            error << "Expected " << GetTypeName<T>() << " value for optional parameter -" << GetName();
            error << "." << std::endl;
        }
        else if (m_isActivated)
        {
            success = false;
            error << "Found second instance of optional argument ";
            Syntax(error);
            error << "." << std::endl;
        }
        else
        {
            if (ParameterBase::TryParse(argv[currentArg], m_value))
            {
                ++currentArg;

                IValidator<T>* validator = m_validator.get();
                if (validator != NULL)
                {
                    success = validator->IsValid(m_value);
                    if (!success)
                    {
                        error << "Validation error: ";
                        error << GetName() << " ";
                        validator->Description(error);
                        error << "." << std::endl;
                    }
                }
                else
                {
                    success = true;
                }
            }
            else
            {
                error << "Expected " << GetTypeName<T>() << " value for optional parameter -" << GetName();
                error << "." << std::endl;
            }
        }

        m_isActivated = success;
        return success;
    }

    template <class T>
    void OptionalParameter<T>::Syntax(std::ostream& out) const
    {
        out << "[-";
        out << GetName();
        out << " <" << GetTypeName<T>() << ">]";
    }

    template <class T>
    void OptionalParameter<T>::Description(std::ostream& out) const
    {
        ParameterBase::Description(out);
        out << " (" << GetTypeName<T>();
        if (HasDefaultValue())
        {
            out << ", defaults to ";
            Format(out, GetDefaultValue());
        }
        if (m_validator.get() != NULL)
        {
            out << ", ";
            m_validator.get()->Description(out);
        }
        out << ")";;
    }

    template <class T>
    std::ostream& operator<<(std::ostream& out, const OptionalParameter<T>& value)
    {
        ParameterBase::Format(out, value.GetValue());
        return out;
    }

}
