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


#include "AnalyzeCommand.h"
#include "BitFunnel/Plan/Factories.h"
#include "Environment.h"
#include "LoggerInterfaces/Check.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // Analyze - analyze row table densities.
    //
    //*************************************************************************
    Analyze::Analyze(Environment & environment,
                     Id id,
                     char const * /*parameters*/)
        : TaskBase(environment, id, Type::Synchronous)
    {
    }


    void Analyze::Execute()
    {
        CHECK_NE(*GetEnvironment().GetOutputDir().c_str(), '\0')
            << "Output directory not set. "
            << "Please use the 'cd' command to set an "
            << "output directory";

        Factories::AnalyzeRowTables(GetEnvironment().GetSimpleIndex(),
                                    GetEnvironment().GetOutputDir().c_str());
    }


    ICommand::Documentation Analyze::GetDocumentation()
    {
        return Documentation(
            "analyze",
            "Analyzes RowTables statistics (e.g. row and column densities).",
            "analyze\n"
            "  Analyzes RowTables statistics (e.g. row and column densities).\n"
        );
    }


}
