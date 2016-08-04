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

#include "BitFunnel/Configuration/Factories.h"
#include "BitFunnel/Exceptions.h"
#include "ShardDefinition.h"


namespace BitFunnel
{
    std::unique_ptr<IShardDefinition> Factories::CreateShardDefinition()
    {
        return std::unique_ptr<IShardDefinition>(new ShardDefinition());
    }


    std::unique_ptr<IShardDefinition> Factories::CreateShardDefinition(std::istream& input)
    {
        return std::unique_ptr<IShardDefinition>(new ShardDefinition(input));
    }


    ShardDefinition::ShardDefinition()
    {
    }


    ShardDefinition::ShardDefinition(std::istream& /*input*/)
    {
    }


    void ShardDefinition::Write(std::ostream& /*output*/) const
    {
        throw NotImplemented();
    }


    void ShardDefinition::AddShard(size_t maxPostingCount)
    {
        m_maxPostingCounts.push_back(maxPostingCount);
        size_t i = m_maxPostingCounts.size() - 1;

        while (i > 0 && m_maxPostingCounts[i] < m_maxPostingCounts[i - 1])
        {
            std::swap(m_maxPostingCounts[i], m_maxPostingCounts[i - 1]);
            --i;
        }
    }


    ShardId ShardDefinition::GetShard(size_t postingCount) const
    {
        size_t i = 0;
        for (; i < m_maxPostingCounts.size(); ++i)
        {
            if (postingCount <= m_maxPostingCounts[i])
            {
                break;
            }
        }
        return static_cast<ShardId>(i);
    }


    size_t ShardDefinition::GetMaxPostingCount(ShardId shard) const
    {
        return m_maxPostingCounts[shard];
    }


    ShardId ShardDefinition::GetShardCount() const
    {
        return static_cast<ShardId>(m_maxPostingCounts.size() + 1);
    }
}
