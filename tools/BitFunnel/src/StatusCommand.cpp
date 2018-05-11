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
#include "BitFunnel/Index/IShard.h"
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
                   char const * parameters)
        : TaskBase(environment, id, Type::Synchronous),
          m_shard(0)
    {
        auto tokens = TaskFactory::Tokenize(parameters);
        if (tokens.size() > 0)
        {
            m_shard = std::stoull(tokens[0].c_str());
        }
    }


    void Status::Execute()
    {
        std::cout
            << "Printing system status ..."
            << std::endl;

        GetEnvironment().GetIngestor().PrintStatistics(std::cout, 0.0);

        std::cout << std::endl;

        std::cout << "SHARD " << m_shard << "-->" << std::endl << std::endl;

        double bytesPerDocument = 0;
        for (Rank rank = 0; rank < c_maxRankValue; ++rank)
        {
            bytesPerDocument += GetEnvironment().GetTermTable(m_shard).GetBytesPerDocument(rank);
            std::cout
                << rank
                << ": "
                << GetEnvironment().GetTermTable(m_shard).GetBytesPerDocument(rank)
                << " bytes/document"
                << std::endl;
        }
        std::cout << std::endl;

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
                << GetEnvironment().GetTermTable(m_shard).GetTotalRowCount(rank)
                << " rows."
                << std::endl;
        }
        std::cout << std::endl;

        double bitsPerDocument = 0.0;
        double documentsPerColumnAtRank = 1.0;
        for (Rank rank = 0; rank < c_maxRankValue; ++rank)
        {
            bitsPerDocument += GetEnvironment().GetTermTable(m_shard).GetTotalRowCount(rank) * documentsPerColumnAtRank;
            documentsPerColumnAtRank /= 2;
        }
        std::cout << "Bits per document: " << bitsPerDocument << std::endl;

        // TODO:  This is document count for corpus vs. shard. totalBits will be incorrect.
        const auto documentCount = GetEnvironment().GetIngestor().GetDocumentCount();
        std::cout << "Document count: " << documentCount << std::endl;

        const double totalBits = documentCount * bitsPerDocument;
        std::cout << "Total bits: " << totalBits << std::endl;

        // TODO: This is posting count for corpus vs. shard. bitsPerPosting will be incorrect.
        const auto postingCount = GetEnvironment().GetIngestor().GetPostingCount();
        std::cout << "Posting count: " << postingCount << std::endl;

        if (postingCount > 0)
        {
            const double bitsPerPosting = totalBits / postingCount;
            std::cout << "Bits per posting: " << bitsPerPosting << std::endl;
        }

        std::cout << std::endl;

        std::cout
            << "Slice capacity: "
            << GetEnvironment().GetIngestor().GetShard(m_shard).GetSliceCapacity()
            << std::endl;
        std::cout << std::endl;
    }


    ICommand::Documentation Status::GetDocumentation()
    {
        return Documentation(
            "status",
            "Prints system status.",
            "status [shard = 0]\n"
            "  Prints system status."
            "  NOT IMPLEMENTED\n"
        );
    }
}
