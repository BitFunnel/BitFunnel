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

#include <iomanip>
#include <iostream>

#include "BitFunnel/BitFunnelTypes.h"
#include "BitFunnel/Exceptions.h"
#include "BitFunnel/Index/IDocument.h"
#include "BitFunnel/Index/IDocumentCache.h"
#include "BitFunnel/Index/IIngestor.h"
#include "BitFunnel/Index/IShard.h"
#include "BitFunnel/Index/RowIdSequence.h"
#include "Environment.h"
#include "ShowCommand.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // Show
    //
    //*************************************************************************
    Show::Show(Environment & environment,
               Id id,
               char const * parameters)
        : TaskBase(environment, id, Type::Synchronous)
    {
        auto tokens = TaskFactory::Tokenize(parameters);
        std::string command;
        
        if (tokens.size() > 0)
        {
            command = tokens[0];
        }

        if (command.compare("cache") == 0)
        {
            m_mode = Mode::Cache;
            if (tokens.size() < 2)
            {
                RecoverableError error("`show cache` expects a term.");
                throw error;
            }
            m_term = tokens[1];
        }
        else if (command.compare("rows") == 0)
        {
            m_mode = Mode::Rows;
            if (tokens.size() < 2)
            {
                RecoverableError error("`show rows` expects a term.");
                throw error;
            }
            m_term = tokens[1];
        }
        else if (command.compare("term") == 0)
        {
            m_mode = Mode::Term;
            if (tokens.size() < 2)
            {
                RecoverableError error("`show term` expects a term.");
                throw error;
            }
            m_term = tokens[1];
        }
        else
        {
            RecoverableError error("`show` command expects \"cache\", \"term\" or \"rows\".");
            throw error;
        }
    }


    void Show::Execute()
    {
        std::ostream& output = GetEnvironment().GetOutputStream();

        if (m_mode == Mode::Cache)
        {
            auto & environment = GetEnvironment();
            Term term(m_term.c_str(), 0, environment.GetConfiguration());
            auto & cache = environment.GetIngestor().GetDocumentCache();

            output << "DocId, Contains" << std::endl;
            for (auto entry : cache)
            {
                output
                    << "  DocId(" << entry.second << ") ";
                if (entry.first.Contains(term))
                {
                    output << "contains ";
                }
                else
                {
                    output << "does not contain ";
                }
                output << m_term << std::endl;
            }
        }
        else
        {
            // TODO: Consider parsing phrase terms here.
            auto & environment = GetEnvironment();
            Term term(m_term.c_str(), 0, environment.GetConfiguration());

            output
                << "Term("
                << "\"" << m_term << "\""
                << ")" << std::endl;

            IIngestor & ingestor = GetEnvironment().GetIngestor();

            auto maxshard = environment.GetMaxShard();
            for (auto shardId = environment.GetMinShard(); shardId <= maxshard; ++shardId)
            {

                output
                    << "Shard: " << shardId << std::endl;

                // Gather DocIds for first 64 documents in shard
                IShard& shard = ingestor.GetShard(shardId);
                std::vector<DocId> ids;
                auto itPtr = shard.GetIterator();
                auto & it = *itPtr;
                for (size_t i = 0; i < 64 && !it.AtEnd(); ++it, ++i)
                {
                    ids.push_back((*it).GetDocId());
                }

                // Print out 100s digit of DocId.
                output << "                 d ";
                for (auto id : ids)
                {
                    output << ((id / 100) % 10);
                }
                output << std::endl;

                // Print ouf 10s digit of DocId.
                output << "                 o ";
                for (auto id : ids)
                {
                    output << ((id / 10) % 10);
                }
                output << std::endl;

                // Print out 1s digit of DocId.
                output << "                 c ";
                for (auto id : ids)
                {
                    output << (id % 10);
                }
                output << std::endl;

                // Print out RowIds and their bits.
                RowIdSequence rows(term, environment.GetTermTable(shardId));
                for (auto row : rows)
                {
                    output
                        << "  RowId("
                        << row.GetRank()
                        << ", "
                        << std::setw(5)
                        << row.GetIndex()
                        << ")";

                    if (m_mode == Mode::Rows)
                    {
                        output << ": ";
                        for (auto id : ids)
                        {
                            if (ingestor.Contains(id))
                            {
                                auto handle = ingestor.GetHandle(id);
                                output << (handle.GetBit(row) ? "1" : "0");
                            }
                        }
                    }

                    output << std::endl;
                }
            }
        }
    }


    ICommand::Documentation Show::GetDocumentation()
    {
        return Documentation(
            "show",
            "Shows information about various data structures.",
            "show cache <term>\n"
            "   | rows <term>\n"
            "   | term <term>\n"
            "  Shows information about various data structures.\n"
            "  Results are shown for the shard(s) specified by `shard`.\n"
        );
    }
}
