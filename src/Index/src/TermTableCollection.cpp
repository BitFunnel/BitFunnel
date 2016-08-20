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

#include <istream>

#include "BitFunnel/IFileManager.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/ITermTable2.h"
#include "TermTable.h"
#include "TermTableCollection.h"


namespace BitFunnel
{
    std::unique_ptr<ITermTableCollection>
        Factories::CreateTermTableCollection(ShardId shardCount)
    {
        return std::unique_ptr<ITermTableCollection>(
            new TermTableCollection(shardCount));
    }


    std::unique_ptr<ITermTableCollection>
        Factories::CreateTermTableCollection(IFileManager & fileManager, ShardId shardCount)
    {
        return std::unique_ptr<ITermTableCollection>(
            new TermTableCollection(fileManager, shardCount));
    }


    TermTableCollection::TermTableCollection(ShardId shardCount)
    {
        for (ShardId shard = 0; shard < shardCount; ++shard)
        {
            std::unique_ptr<ITermTable2> termTable(new TermTable());

            // TermTable must be sealed before it can be used.
            termTable->Seal();

            m_termTables.push_back(std::move(termTable));
        }
    }


    TermTableCollection::TermTableCollection(IFileManager & fileManager,
                                             ShardId shardCount)
    {
        for (ShardId shard = 0; shard < shardCount; ++shard)
        {
            auto input = fileManager.TermTable(0).OpenForRead();
            m_termTables.emplace_back(
                std::unique_ptr<ITermTable2>(new TermTable(*input)));
        }
    }


    ITermTable2 & TermTableCollection::GetTermTable(ShardId shard) const
    {
        return *m_termTables.at(shard);
    }

    
    size_t TermTableCollection::size() const
    {
        return m_termTables.size();
    }
}
