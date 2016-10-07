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

#include "BitFunnel/NonCopyable.h"                  // Inherits from NonCopyable.
#include "BitFunnel/Utilities/IPersistableObject.h" // Inherits from IPersistableObject.


namespace BitFunnel
{
    class IObjectFormatter;
    class IObjectParser;
    class IPlanRows;
    class RowMatchNode;


    class RowPlanBase : public IPersistableObject,
                        protected NonCopyable
    {
    public:
        enum NodeType
        {
            Invalid = -2,
            Null = -1,

            // DESIGN NOTE: legal node types have consecutive values starting
            // at zero so that TypeCount is equal to the number of legal nodes.

            // Match
            AndMatch = 0,
            NotMatch,
            OrMatch,
            ReportMatch,
            RowMatch,

            // Plan
            Plan,

            // Total number of node types.
            TypeCount
        };

        virtual NodeType GetType() const = 0;

        //
        // IPersistableObject methods
        //
        int GetTypeTag() const;
        const char* GetTypeName() const;

        //
        // Static methods
        //
        static int GetType(const char*);
    };


    class RowPlan : public RowPlanBase
    {
    public:
        RowPlan(RowMatchNode const & matchTree,
                IPlanRows const & planRows);

        RowPlan(IObjectParser& parser);

        void Format(IObjectFormatter& formatter) const;

        RowMatchNode const & GetMatchTree() const;
        IPlanRows const & GetPlanRows() const;

        NodeType GetType() const;

        static RowPlan& Parse(IObjectParser& parser);

    private:
        // WARNING: The persistence format depends on the order in which the
        // following three members are declared. If the order is changed, it is
        // neccesary to update the corresponding code in the RowPlan::RowPlan()
        // and RowPlan::Format().
        RowMatchNode const & m_matchTree;
        IPlanRows const & m_planRows;
    };
}
