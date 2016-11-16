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

#include <stddef.h>     // size_t paramter.
#include <vector>       // std::vector parameter.


namespace BitFunnel
{
    class ICostFunction;

    namespace SingleSourceShortestPath
    {
        //*********************************************************************
        //
        // FindPath finds the minimum weight path between two vertices in a
        // a topologically sorted directed acyclic graph. The graph structure
        // and edge weights are provided by the ICostFunction parameter.
        // ICostFunction also provides the start point vertex and endpoint
        // vertex. The minimum weight path is returned as a vector of vertex
        // numbers. See the comment in ICostFunction.h for more information
        // about the representation of the graph.
        //
        //*********************************************************************
        void FindPath(ICostFunction& costFunction,
                      size_t maxPathLength,
                      std::vector<size_t>& path);
    }
}
