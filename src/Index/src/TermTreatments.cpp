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

#include <iostream>             // TODO: Remove this temporary include.

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

        std::cout << "Single configuration: ";
        m_configuration.Write(std::cout);
        std::cout << std::endl;
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
        double maxSharedRowFrequency = density / 10.0;

        // TODO: fill up vector of RowConfigurations.
        for (Term::IdfX10 idf = 0; idf < 90; ++idf)
        {
            RowConfiguration configuration;

            double frequency = pow(10, -idf / 10.0);        // TODO: use method from Term.cpp.
            //std::cout << "idf = " << (float)idf << ", ";
            //std::cout << "frequency = " << frequency << std::endl;
            if (frequency > maxSharedRowFrequency)
            {
                configuration.push_front(RowConfiguration::Entry(0, 1, true));
            }
            else
            {
                unsigned k = lround(ceil(log(frequency / snr) / log(density - frequency)) + 1);
                configuration.push_front(RowConfiguration::Entry(0, k, false));
            }

            m_configurations.push_back(configuration);

            std::cout << idf / 10.0 << ": ";
            m_configurations.back().Write(std::cout);
            std::cout << std::endl;
        }
    }


    RowConfiguration TreatmentPrivateSharedRank0::GetTreatment(Term term) const
    {
        return m_configurations[term.GetIdfSum()];
    }


    //*************************************************************************
    //
    // TreatmentPrivateSharedRank0and3
    //
    // Terms get one or more rank 0 and rank 3 rows that could be private or
    // shared, depending on term frequency.
    //
    //*************************************************************************
    TreatmentPrivateSharedRank0and3::TreatmentPrivateSharedRank0and3(double density, double snr)
    {
        double maxSharedRowFrequency = density / 10.0;

        // TODO: fill up vector of RowConfigurations.
        for (Term::IdfX10 idf = 0; idf < 90; ++idf)
        {
            RowConfiguration configuration;

            double frequency = pow(10, -idf / 10.0);    // TODO: Use method from term.cpp. Use IdfX10.
            if (frequency > maxSharedRowFrequency)
            {
                configuration.push_front(RowConfiguration::Entry(0, 1, true));
            }
            else
            {
                unsigned k = lround(ceil(log(frequency / snr) / log(density - frequency)) + 1);
                configuration.push_front(RowConfiguration::Entry(0, 1, false));
                if (k > 1)
                {
                    // TODO: compute frequency at rank.
                    Rank rank = 3;
                    double frequencyAtRank = 1 - pow(1.0 - frequency, rank + 1);    // TODO: Put this in shared function.
                    if (frequencyAtRank > maxSharedRowFrequency)
                    {
                        configuration.push_front(RowConfiguration::Entry(3, 1, true));
                    }
                    else
                    {
                        configuration.push_front(RowConfiguration::Entry(3, k - 1, false));
                    }
                }
            }

            m_configurations.push_back(configuration);

            std::cout << idf / 10.0 << ": ";
            m_configurations.back().Write(std::cout);
            std::cout << std::endl;
        }
    }


    RowConfiguration TreatmentPrivateSharedRank0and3::GetTreatment(Term term) const
    {
        return m_configurations[term.GetIdfSum()];
    }
}
