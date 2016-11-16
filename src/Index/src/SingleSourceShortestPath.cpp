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


#include <limits>   // std::numeric_limits<float>::infinity().

#include "ICostFunction.h"
#include "LoggerInterfaces/Check.h"
#include "SingleSourceShortestPath.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // FindPath finds the minimum weight path between two vertices in a
    // topologically sorted directed acyclic graph. A topologically sorted
    // graph is one where the vertices have been ordered such that edges only
    // run from earlier vertices to later vertices. The first vertex in a
    // topoloically sorted graph has no incoming edges and the last vertex has
    // no outgoing edges.
    //
    // The graph structure and edge weights are provided by the ICostFunction
    // parameter. ICostFunction also provides the start point vertex and
    // endpoint vertex for the path computation. See the comment in
    // ICostFunction.h for more information about the representation of the
    // graph.
    //
    // The minimum weight path is returned as a vector of vertex numbers. Note
    // that FindPath() will not clear the path vector. This allows the caller
    // to fill the vector with a path fragment and then call FindPath() to
    // append another path. This has practical utility when using special logic
    // to construct the path from node 0 to the start vertex.
    //
    // The algorithm maintains a vector, indexed by vertex number, where each
    // entry contains the cost of the best known path from the start vertex.
    // Initially every entry in this array is set to positive infinity.
    //
    // The algorithm then visits the vertices in topological order. At each,
    // vertex, outgoing edges are enumerated. Each edge provides a weight
    // which is added to the current vertex's path cost. This value is the
    // cost of the best path from the start vertex through the current vertex
    // to the edge's endpoint. This value is written into the enpoint's slot
    // in the cost vector if it is less than that vertex's current best path
    // cost. This continues until all of the vertices have been visited. At
    // this point, best path costs have propagated from the start vertex
    // through all other vertices.
    //
    // To assist in identifying the lowest cost path, the algorithm also
    // maintains a vector of vertex numbers which is used to record for each
    // vertex, the number of the previous vertex on the lowest cost path.
    // Entries in this array are updated whenever a value is written to the
    // cost vector.
    //
    // This algorithm is called "Single-source shortest paths in directed
    // acyclic graphs." A great reference is "Introduction to Algorithms" by
    // Cormen, Leiserson, and Rivest. In the Sixth Printing from 1992, see
    // Chapter 25.4, pages 536-538. The Wikipedia entry for the book is at
    // http://en.wikipedia.org/wiki/Introduction_to_Algorithms.
    //
    // There are also numerous descriptions of this algorithm and proofs
    // of its correctness on the web.
    //
    //*********************************************************************
    void SingleSourceShortestPath::FindPath(ICostFunction& costFunction,
                                            size_t maxPathLength,
                                            std::vector<size_t>& pathOut)
    {
        const size_t c_invalidVertex =
            std::numeric_limits<size_t>::max();

        // Can't find a path unless there is at least one edge or two vertices.
        CHECK_GE(costFunction.GetVertexCount(), 2)
            << "Cost function must include at least two vertices.";

        //
        // Initialize vectors that hold shortest path costs and previous
        // vertices.
        //
        size_t vertexCount = costFunction.GetVertexCount();
        std::vector<std::vector<float>>
            shortestPathCost(vertexCount,
                             std::vector<float>(maxPathLength + 1,
                                                std::numeric_limits<float>::infinity()));
        std::vector<std::vector<size_t>>
            previousVertex(vertexCount,
                           std::vector<size_t>(maxPathLength + 1, c_invalidVertex));

        //
        // Propagate shortest path costs from startVertex through all
        // intermediate vertices to endVertex.
        //

        // The cost of a zero-length path is 0.
        shortestPathCost[0][0] = 0;
        previousVertex[0][0] = 0;

        // Enumerate vertices between the start and end vertex in topological
        // order.
        for (size_t startVertex = 0; startVertex < vertexCount - 1; ++startVertex)
        {
            // Set the start vertex. Need to add firstPathVertex to convert
            // from position along path to graph vertex number.
            costFunction.StartAt(startVertex);

            // Enumerate all outgoing edges. Since the graph is topologically
            // sorted, outgoing edges can only go to vertices with higher
            // numbers, so start endVertex at startVertex + 1.
            for (size_t endVertex = startVertex + 1; endVertex < vertexCount; ++endVertex)
            {
                // Advance the cost function to the next vertex in topological
                // order. The cost function should now be configured to compute
                // the cost between the start vertex and the end vertex.
                costFunction.Extend();

                for (size_t pathLength = 1; pathLength <= maxPathLength; pathLength++)
                {
                    // Compute the new cost for a path through the outgoing edge.
                    if (previousVertex[startVertex][pathLength - 1] == c_invalidVertex)
                    {
                        continue;
                    }

                    float newCost = shortestPathCost[startVertex][pathLength - 1] + costFunction.GetCost();

                    // If the new cost is better than the best known cost, update
                    // the best known cost and the previous vertex.
                    if (newCost < shortestPathCost[endVertex][pathLength]) 
                    {
                        shortestPathCost[endVertex][pathLength] = newCost;
                        previousVertex[endVertex][pathLength] = startVertex;
                    }
                }
            }
        }

        // Find path with lowest cost
        size_t currentVertex = vertexCount - 1;

        float minCost = std::numeric_limits<float>::infinity();
        size_t minCostPathLength = 0;
        for (size_t pathLength = 0; pathLength <= maxPathLength; pathLength++)
        {
            if (previousVertex[currentVertex][pathLength] == c_invalidVertex)
            {
                continue;
            }

            if (shortestPathCost[currentVertex][pathLength] < minCost)
            {
                minCost = shortestPathCost[currentVertex][pathLength];
                minCostPathLength = pathLength;
            }
        }

        //
        // Construct reversed path.
        //

        // Storage for the best path. This path will be appended to pathOut.
        std::vector<size_t> path;

        // Push the path vertices.
        path.push_back(currentVertex);
        while (currentVertex != 0)
        {
            currentVertex = previousVertex[currentVertex][minCostPathLength--];
            path.push_back(currentVertex);
        }
        CHECK_EQ(minCostPathLength, 0u);

        // Path is now in reverse order. Bring it into the correct order while
        // appending it to pathOut.
        const size_t lastVertex = path.size() - 1;
        for (size_t i = 0 ; i <= lastVertex; ++i)
        {
            pathOut.push_back(path[lastVertex - i]);
        }
    }
}
