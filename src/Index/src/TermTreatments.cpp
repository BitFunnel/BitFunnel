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

#include <algorithm>
#include <iostream>             // TODO: Remove this temporary include.
#include <math.h>

#include "BitFunnel/Term.h"
#include "TermTreatments.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // TreatmentPrivateRank0
    //
    // All terms get the same treatment - a single, private, rank 0 row.
    //
    //*************************************************************************
    TreatmentPrivateRank0::TreatmentPrivateRank0()
    {
        // Same configuration for all terms - one private rank 0 row.
        m_configuration.push_front(RowConfiguration::Entry(0, 1, true));

        //std::cout << "Single configuration: ";
        //m_configuration.Write(std::cout);
        //std::cout << std::endl;
    }


    RowConfiguration TreatmentPrivateRank0::GetTreatment(Term /*term*/) const
    {
        return m_configuration;
    }


    //*************************************************************************
    //
    // TreatmentPrivateSharedRank0
    //
    // Terms get one or more rank 0 rows that could be private or shared,
    // depending on term frequency.
    //
    //*************************************************************************
    TreatmentPrivateSharedRank0::TreatmentPrivateSharedRank0(double density, double snr)
    {
        // Fill up vector of RowConfigurations. GetTreatment() will use the
        // IdfSum() value of the Term as an index into this vector.
        for (Term::IdfX10 idf = 0; idf <= Term::c_maxIdfX10Value; ++idf)
        {
            RowConfiguration configuration;

            double frequency = Term::IdfX10ToFrequency(idf);
            if (frequency >= density)
            {
                // This term is so common that it must be assigned a private row.
                configuration.push_front(RowConfiguration::Entry(0, 1, true));
            }
            else
            {
                // Determine the number of rows, k, required to reach the
                // desired signal to noise ratio, snr, given a certain bit
                // density.
                unsigned k = lround(ceil(log(frequency / snr) / log(density - frequency)) + 1);
                configuration.push_front(RowConfiguration::Entry(0, k, false));
            }

            m_configurations.push_back(configuration);

            //std::cout << idf / 10.0 << ": ";
            //m_configurations.back().Write(std::cout);
            //std::cout << std::endl;
        }
    }


    RowConfiguration TreatmentPrivateSharedRank0::GetTreatment(Term term) const
    {
        Term::IdfX10 idf = std::min(term.GetIdfSum(), Term::c_maxIdfX10Value);
        return m_configurations[idf];
    }


    //*************************************************************************
    //
    // TreatmentPrivateSharedRank0And3
    //
    // Terms get one or more rank 0 and rank 3 rows that could be private or
    // shared, depending on term frequency.
    //
    //*************************************************************************
    TreatmentPrivateSharedRank0And3::TreatmentPrivateSharedRank0And3(double density, double snr)
    {
        // Fill up vector of RowConfigurations. GetTreatment() will use the
        // IdfSum() value of the Term as an index into this vector.
        for (Term::IdfX10 idf = 0; idf <= Term::c_maxIdfX10Value; ++idf)
        {
            RowConfiguration configuration;

            double frequency = Term::IdfX10ToFrequency(idf);
            if (frequency > density)
            {
                // This term is so common that it must be assigned a private row.
                configuration.push_front(RowConfiguration::Entry(0, 1, true));
            }
            else
            {
                // Determine the number of rows, k, required to reach the
                // desired signal to noise ratio, snr, given a certain bit
                // density.
                unsigned k = lround(ceil(log(frequency / snr) / log(density - frequency)) + 1);
                configuration.push_front(RowConfiguration::Entry(0, 1, false));
                if (k > 1)
                {
                    Rank rank = 3;
                    double frequencyAtRank = Term::FrequencyAtRank(frequency, rank);
                    if (frequencyAtRank >= density)
                    {
                        configuration.push_front(RowConfiguration::Entry(rank, 1, true));
                    }
                    else
                    {
                        configuration.push_front(RowConfiguration::Entry(rank, k - 1, false));
                    }
                }
            }

            m_configurations.push_back(configuration);

            //std::cout << idf / 10.0 << ": ";
            //m_configurations.back().Write(std::cout);
            //std::cout << std::endl;
        }
    }


    RowConfiguration TreatmentPrivateSharedRank0And3::GetTreatment(Term term) const
    {
        Term::IdfX10 idf = std::min(term.GetIdfSum(), Term::c_maxIdfX10Value);
        return m_configurations[idf];
    }
}
