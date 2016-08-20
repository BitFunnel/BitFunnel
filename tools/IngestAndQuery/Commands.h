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

#include "TaskBase.h"   // TaskBase base class.


namespace BitFunnel
{
    class DelayedPrint : public TaskBase
    {
    public:
        DelayedPrint(Environment & environment,
                     Id id,
                     char const * parameters);

        virtual void Execute() override;
        static ICommand::Documentation GetDocumentation();

    private:
        size_t m_sleepTime;
        std::string m_message;
    };


    class Exit : public TaskBase
    {
    public:
        Exit(Environment & environment,
             Id id,
             char const * parameters);

        virtual void Execute() override;
        static ICommand::Documentation GetDocumentation();
    };


    class Help : public TaskBase
    {
    public:
        Help(Environment & environment,
             Id id,
             char const * parameters);

        virtual void Execute() override;
        static ICommand::Documentation GetDocumentation();

    private:
        std::string m_command;
    };


    class Ingest : public TaskBase
    {
    public:
        Ingest(Environment & environment,
               Id id,
               char const * parameters);

        virtual void Execute() override;
        static ICommand::Documentation GetDocumentation();

    private:
        bool m_manifest;
        std::string m_path;
    };


    class Query : public TaskBase
    {
    public:
        Query(Environment & environment,
              Id id,
              char const * parameters);

        virtual void Execute() override;
        static ICommand::Documentation GetDocumentation();

    private:
        bool m_isSingleQuery;
        std::string m_query;
    };


    class Script : public TaskBase
    {
    public:
        Script(Environment & environment,
               Id id,
               char const * parameters);

        virtual void Execute() override;
        static ICommand::Documentation GetDocumentation();

    private:
        std::vector<std::string> m_script;
    };


    class Show : public TaskBase
    {
    public:
        Show(Environment & environment,
             Id id,
             char const * parameters);

        virtual void Execute() override;
        static ICommand::Documentation GetDocumentation();

        enum class Mode
        {
            Term,
            Rows
        };

    private:
        Mode m_mode;
        std::string m_term;
    };


    class Status : public TaskBase
    {
    public:
        Status(Environment & environment,
               Id id,
               char const * parameters);

        virtual void Execute() override;
        static ICommand::Documentation GetDocumentation();
    };
}
