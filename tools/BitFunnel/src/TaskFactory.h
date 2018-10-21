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

#include <iosfwd>                   // std::ostream parameter.
#include <map>                      // std::map embedded.
#include <memory>                   // std::unique_ptr return value.
#include <string>                   // std::string return value.
#include <vector>                   // std::vector return value.

#include "BitFunnel/IInterface.h"   // Base class.
#include "BitFunnel/NonCopyable.h"  // Base class.
#include "ICommand.h"               // Base class.


namespace BitFunnel
{
    class Environment;

    class TaskFactory : public NonCopyable
    {
    public:
        TaskFactory(Environment & environment);
        virtual ~TaskFactory();

        virtual std::unique_ptr<ICommand> CreateTask(char const * line);
        virtual void Help(std::ostream& output, char const * command) const;
        ICommand::Id GetNextTaskId() const;

    private:
        class Descriptor;

    public:
        template <class T>
        void RegisterCommand()
        {
            std::unique_ptr<Descriptor> descriptor(
                new Descriptor(T::GetDocumentation(),
                               Create<T>));
            RegisterHelper(std::move(descriptor));
        }

        static std::vector<std::string> Tokenize(char const * text);
        static std::string GetNextToken(char const * & text);

    private:
        void RegisterHelper(std::unique_ptr<Descriptor> descriptor);

        Environment & m_environment;
        ICommand::Id m_nextId;
        // m_maxNameLength is an int because setw takes an int.
        int m_maxNameLength;
        std::map<std::string, std::unique_ptr<Descriptor>> m_taskMap;


        typedef std::unique_ptr<ICommand>(*Creator)(
            Environment & environment,
            ICommand::Id id,
            char const * parameters);


        class Descriptor
        {
        public:
            Descriptor(ICommand::Documentation documentation,
                       Creator creator)
              : m_documentation(documentation),
                m_creator(creator)
            {
            }

            ICommand::Documentation GetDocumentation() const
            {
                return m_documentation;
            }


            std::unique_ptr<ICommand> Create(
                Environment & environment,
                ICommand::Id id,
                char const * parameters)
            {
                return m_creator(environment,
                                 id,
                                 parameters);
            }

        private:
            ICommand::Documentation m_documentation;
            Creator m_creator;
        };


        template <class T>
        static std::unique_ptr<ICommand> Create(
            Environment & environment,
            ICommand::Id id,
            char const * parameters)
        {
            return std::unique_ptr<ICommand>(new T(environment, id, parameters));
        }
    };
}
