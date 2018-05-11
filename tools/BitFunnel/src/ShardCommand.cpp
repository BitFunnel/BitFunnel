// The MIT License (MIT)

// Copyright (c) 2018, Microsoft

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

#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Index/IIngestor.h"
#include "Environment.h"
#include "ShardCommand.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // Shard - set shard(s) to show results for
    //
    //*************************************************************************
    ShardCommand::ShardCommand(Environment & environment,
           Id id,
           char const * parameters)
        : TaskBase(environment, id, Type::Synchronous)
    {
        auto tokens = TaskFactory::Tokenize(parameters);
        if (tokens.size() > 0)
        {
            if (tokens[0].compare("all") == 0)
            {
                m_minshard = 0;
                m_maxshard = environment.GetIngestor().GetShardCount() - 1;
            }
            else
            {
                m_minshard = std::stoull(tokens[0]);
                if (tokens.size() > 1)
                {
                    m_maxshard = std::stoull(tokens[1]);
                    if (m_minshard > m_maxshard)
                    {
                        RecoverableError error("maxshard must not be less than minshard.");
                        throw error;
                    }
                }
                else
                {
                    m_maxshard = m_minshard;
                }
            }
        }
        else
        {
            RecoverableError error("`shard` expects `all` or a shard number/range.");
            throw error;
        }
    }


    void ShardCommand::Execute()
    {
        GetEnvironment().SetShards(m_minshard, m_maxshard);
        if (m_minshard == m_maxshard)
        {
            std::cout
                << "Now showing results for shard " << m_minshard
                << "." << std::endl;
        }
        else
        {
            std::cout
                << "Now showing results for shards "
                << m_minshard << "-" << m_maxshard
                << "." << std::endl;
        }
    }


    ICommand::Documentation ShardCommand::GetDocumentation()
    {
        return Documentation(
            "shard",
            "Set shard(s) to show results for (e.g., 'show rows')",
            "shard <minshard> <maxshard>\n"
            "  Changes the shard(s) to show results for.\n"
            "  <minshard> may be \"all\" to show results for all shards.\n"
            "  <maxshard> is optional and may be omitted.\n"
        );
    }
}
