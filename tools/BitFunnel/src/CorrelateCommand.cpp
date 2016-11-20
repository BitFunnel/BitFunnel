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

#include <iostream>  // TODO: remove.

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Configuration/IFileSystem.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Utilities/ReadLines.h"
#include "CorrelateCommand.h"
#include "Environment.h"
#include "LoggerInterfaces/Check.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // Correlate - analyze row table densities.
    //
    //*************************************************************************
    Correlate::Correlate(Environment & environment,
                     Id id,
                     char const * parameters)
        : TaskBase(environment, id, Type::Synchronous)
    {
        std::string termsFilename = TaskFactory::GetNextToken(parameters);
        auto fileSystem = Factories::CreateFileSystem();  // TODO: Use environment file system
        m_terms = ReadLines(*fileSystem, termsFilename.c_str());
    }


    void Correlate::Execute()
    {
        CHECK_NE(*GetEnvironment().GetOutputDir().c_str(), '\0')
            << "Output directory not set. "
            << "Please use the 'cd' command to set an "
            << "output directory";

        std::cout << "Dummy correlate command." << std::endl;

        Factories::CreateCorrelate(GetEnvironment().GetSimpleIndex(),
                                   GetEnvironment().GetOutputDir().c_str(),
                                   m_terms);
    }


    ICommand::Documentation Correlate::GetDocumentation()
    {
        return Documentation(
            "correlate",
            "Calculate RowTable correlations.",
            "correlate\n"
            "  Calculate RowTable correlations.\n"
        );
    }


}
