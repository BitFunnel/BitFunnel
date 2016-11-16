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

#include <stddef.h>     // size_t parameter.


namespace BitFunnel
{
    //*************************************************************************
    //
    // ICostFunction is an abstract base class or interface for classes that
    // provides the edge weights on a topologically sorted directed acycyclic
    // graph. It is consumed by SingleSourceShortestPath::FindPath() and
    // ShardDefinitionBuilder::CreateShardDefinition().
    //
    // Vertices in the graph are assigned consecutive vertex numbers, starting
    // at zero. Because the graph is topologically sorted, edges can only run
    // from lower-numbered vertices to higher numbered vertices.
    //
    // Edge weights are retrieved with a three step process. First the starting
    // vertex for an edge is specified with the StartAt() method. This method
    // also sets the end vertex to be the same as the start vertex. Once the
    // starting vertex has been set, subsequent calls to Extend() will
    // increment the ending vertex number. A call to GetCost() at any time will
    // return the weight on the edge from the current starting vertex to the
    // current ending vertex. If no such edge exists, GetCost() will return
    // positive infinity.
    //
    // DESIGN NOTE: ICostFunction uses the modal API with StartAt(), Extend(),
    // and GetCost() to allow a performance optmization in the case where the
    // Weght(A --> B + 1) = F(A --> B) + F(B -> B + 1) + G(A) where F() and
    // G() are arbitrary functions. ICostFunction is currently used by a
    // single source shortest path algorithm that makes happens to gather the
    // weights of all outgoing edges from a single vertex in a consecutive
    // calls ordered by increasing destination vertiex, e.g. A --> A + 1, 
    // A --> A + 2, A --> A + 3, etc.
    //
    //*************************************************************************
    class ICostFunction
    {
    public:
        virtual ~ICostFunction() {};

        // Sets the start vertex for subsequent calls to GetCost(). This method
        // also sets the end vertex to the same value.
        virtual void StartAt(size_t vertex) = 0;

        // Increments the end vertex by one.
        virtual void Extend() = 0;

        // Returns the weight of an edge from the start vertex to the end
        // vertex. If no such edge exists, the return value will be positive
        // infinity.
        virtual float GetCost() const = 0;

        // Returns the number vertices in the graph.
        virtual size_t GetVertexCount() const = 0;
    };
}
