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

#include <random>   // Embedded types.
#include <stddef.h> // size_t embedded.
#include <string>   // std::string return value.


namespace BitFunnel
{
    class IDocumentFrequencyTable;
    class ITermToText;


    //*************************************************************************
    //
    // Constructs random conjunction queries where terms are selected from
    // a document frequency table, using a lognormal distribution which biases
    // term selection towards more common terms.
    //
    //*************************************************************************
    class QueryGenerator
    {
    public:
        QueryGenerator(IDocumentFrequencyTable const & dft,
                       ITermToText const & terms);

        std::string CreateOneQuery(size_t termCount);

    private:
        // Selects a random term and appends it to the query string.
        void AppendOneTerm(std::string& query);

        //
        // Constructor parameters
        //

        IDocumentFrequencyTable const & m_dft;
        ITermToText const & m_terms;

        //
        // Random term generation.
        //

        std::default_random_engine m_generator;
        std::lognormal_distribution<double> m_distribution;

        // Number of random numbers to grab at startup. This is effectively
        // the number of terms that can appear in generated queries before
        // reusing a random number.
        // DESIGN NOTE: This algorithm generates the random numbers up front
        // so that it adjust them to exactly cover the number of terms in the
        // document frequency table. See implementation of AppendOneTerm()
        // for more information.
        static const size_t c_valueCount = 10 * 1000 * 1000;

        // Pre-generated random values used to select terms from the document
        // frequency table.
        std::vector<double> m_values;

        // Maximum value in m_values. Used to scale random values to cover all
        // positions in the document frequency table.
        double m_maxValue;

        // Number of random values used so far. Because this value can exceed
        // m_values.size() it is important to mod by m_values.size().
        size_t m_nextValue;
    };
}
