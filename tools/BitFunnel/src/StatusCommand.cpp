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

#include <iostream>

#include "BitFunnel/BitFunnelTypes.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/ITermTable.h"
#include "Environment.h"
#include "StatusCommand.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // Status
    //
    //*************************************************************************
    Status::Status(Environment & environment,
                   Id id,
                   char const * /*parameters*/)
        : TaskBase(environment, id, Type::Synchronous)
    {
    }


    void Status::Execute()
    {
        std::cout
            << "Printing system status ..."
            << std::endl;

        GetEnvironment().GetIngestor().PrintStatistics(std::cout);

        std::cout << std::endl;

        double bytesPerDocument = 0;
        for (Rank rank = 0; rank < c_maxRankValue; ++rank)
        {
            bytesPerDocument += GetEnvironment().GetTermTable().GetBytesPerDocument(rank);
            std::cout
                << rank
                << ": "
                << GetEnvironment().GetTermTable().GetBytesPerDocument(rank)
                << " bytes/document"
                << std::endl;
        }

        std::cout
            << "Total: "
            << bytesPerDocument
            << " bytes/document"
            << std::endl;

        std::cout << std::endl;

        for (Rank rank = 0; rank < c_maxRankValue; ++rank)
        {
            std::cout
                << rank
                << ": "
                << GetEnvironment().GetTermTable().GetTotalRowCount(rank)
                << " rows."
                << std::endl;
        }
    }


    ICommand::Documentation Status::GetDocumentation()
    {
        return Documentation(
            "status",
            "Prints system status.",
            "status\n"
            "  Prints system status."
            "  NOT IMPLEMENTED\n"
        );
    }
}
