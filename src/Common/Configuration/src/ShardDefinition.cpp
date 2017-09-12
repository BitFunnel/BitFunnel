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
#include <ostream>
#include <limits>

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Utilities/Exists.h"
#include "CsvTsv/Csv.h"
#include "LoggerInterfaces/Check.h"
#include "ShardDefinition.h"

#include <iostream>         // Temporary - for debugging.

namespace BitFunnel
{
    std::unique_ptr<IShardDefinition> Factories::CreateShardDefinition()
    {
        return std::unique_ptr<IShardDefinition>(new ShardDefinition());
    }


    std::unique_ptr<IShardDefinition>
        Factories::CreateShardDefinition(std::istream& input)
    {
        return std::unique_ptr<IShardDefinition>(new ShardDefinition(input));
    }


    std::unique_ptr<IShardDefinition>
        BitFunnel::Factories::CreateGeometricShardDefinition(size_t start,
                                                             double growthFactor,
                                                             size_t limit)
    {
        CHECK_GT(growthFactor, 1.0) << "Growth factor too small.";
        std::unique_ptr<IShardDefinition> sd(new ShardDefinition());

        // TODO: Issue #396. Is there some way to provider a density
        // other than the default?
        const double defaultDensity = 0.15;

        sd->AddShard(0, defaultDensity);
        while (start < limit)
        {
            sd->AddShard(start, defaultDensity);
            size_t newStart = static_cast<size_t>(start * growthFactor);
            CHECK_GT(newStart, start) << "Growth factor too small.";
            start = newStart;
        }

        return sd;
    }


    std::unique_ptr<IShardDefinition>
        BitFunnel::Factories::CreateDefaultShardDefinition()
    {
        return CreateGeometricShardDefinition(32, 2.0, 16385);
    }


    std::unique_ptr<IShardDefinition>
        BitFunnel::Factories::LoadOrCreateDefaultShardDefinition(IFileManager & fileManager)
    {
        std::cout << "  Factories::LoadOrCreateDefaultShardDefinition()" << std::endl;
        if (fileManager.ShardDefinition().Exists())
        {
            std::cout << "    ShardDefinition file exists. Load it." << std::endl;
            auto input = fileManager.ShardDefinition().OpenForRead();
            auto shardDefinition = CreateShardDefinition(*input);
            std::cout << "    Shard count == " << shardDefinition->GetShardCount() << std::endl;
            return shardDefinition;
        }
        else
        {
            std::cout << "    ShardDefinition file does not exist. Create default." << std::endl;
            auto shardDefinition = CreateDefaultShardDefinition();
            auto output = fileManager.ShardDefinition().OpenForWrite();
            shardDefinition->Write(*output);
            std::cout << "    Shard count == " << shardDefinition->GetShardCount() << std::endl;
            return shardDefinition;
        }
    }


    //*************************************************************************
    //
    // ShardDefinition
    //
    //*************************************************************************
    ShardDefinition::ShardDefinition()
    {
    }


    ShardDefinition::ShardDefinition(std::istream& input)
    {
        CsvTsv::InputColumn<size_t> 
            minPostings("MinPostings",
                        "Minimum number of postings for any document in shard.");
        CsvTsv::InputColumn<double>
            density("Density",
                    "Target density for RowTables in the shard.");

        CsvTsv::CsvTableParser parser(input);
        CsvTsv::TableReader reader(parser);
        reader.DefineColumn(minPostings);
        reader.DefineColumn(density);

        reader.ReadPrologue();
        while (!reader.AtEOF())
        {
            reader.ReadDataRow();

            AddShard(minPostings, density);
        }
        reader.ReadEpilogue();
    }


    void ShardDefinition::Write(std::ostream& output) const
    {
        CsvTsv::OutputColumn<size_t>
            minPostings("MinPostings",
                        "Minimum number of postings for any document in shard.");
        CsvTsv::OutputColumn<double>
            density("Density",
                    "Target density for RowTables in the shard.");

        CsvTsv::CsvTableFormatter formatter(output);
        CsvTsv::TableWriter writer(formatter);
        writer.DefineColumn(minPostings);
        writer.DefineColumn(density);

        writer.WritePrologue();
        for (unsigned i = 0; i < m_shards.size(); ++i)
        {
            minPostings = m_shards[i].first;
            density = m_shards[i].second;

            writer.WriteDataRow();
        }
        writer.WriteEpilogue();
    }


    void ShardDefinition::AddShard(size_t minPostingCount, double density)
    {
        CHECK_LT(m_shards.size(), c_maxShardIdCount)
            << "Shard count limit exceeded.";

        CHECK_TRUE(m_shards.size() > 0 || minPostingCount == 0)
            << "First shard must have minPostingCount of zero.";

        m_shards.push_back(std::make_pair(minPostingCount, density));
        size_t i = m_shards.size() - 1;

        while (i > 0 && m_shards[i].first < m_shards[i - 1].first)
        {
            std::swap(m_shards[i], m_shards[i - 1]);
            --i;
        }
    }


    ShardId ShardDefinition::GetShard(size_t postingCount) const
    {
        size_t i = 0;
        for (; i < m_shards.size() - 1; ++i)
        {
            if (postingCount < m_shards[i + 1].first)
            {
                break;
            }
        }
        return static_cast<ShardId>(i);
    }


    size_t ShardDefinition::GetMinPostingCount(ShardId shard) const
    {
        return m_shards[shard].first;
    }


    size_t ShardDefinition::GetMaxPostingCount(ShardId shard) const
    {
        if (shard < m_shards.size() - 1)
            return m_shards[shard + 1].first;
        else
            return std::numeric_limits<size_t>::max();
    }


    double ShardDefinition::GetDensity(ShardId shard) const
    {
        return m_shards[shard].second;
    }


    ShardId ShardDefinition::GetShardCount() const
    {
        return static_cast<ShardId>(m_shards.size());
    }
}
