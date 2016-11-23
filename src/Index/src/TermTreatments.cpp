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

#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Term.h"
#include "TermTreatments.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // Factory methods.
    //
    //*************************************************************************
    std::unique_ptr<ITermTreatment> Factories::CreateTreatmentPrivateRank0()
    {
        return std::unique_ptr<ITermTreatment>(new TreatmentPrivateRank0());
    }


    std::unique_ptr<ITermTreatment>
        Factories::CreateTreatmentPrivateSharedRank0(double density,
                                                     double snr)
    {
        return
            std::unique_ptr<ITermTreatment>(
                new TreatmentPrivateSharedRank0(density, snr));
    }


    std::unique_ptr<ITermTreatment>
        Factories::CreateTreatmentPrivateSharedRank0And3(double density,
                                                        double snr)
    {
        return
            std::unique_ptr<ITermTreatment>(
                new TreatmentPrivateSharedRank0And3(density, snr));
    }


    std::unique_ptr<ITermTreatment>
        Factories::CreateTreatmentPrivateSharedRank0ToN(double density,
                                                        double snr)
    {
        return
            std::unique_ptr<ITermTreatment>(
                new TreatmentPrivateSharedRank0ToN(density, snr));
    }


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
                int k = Term::ComputeRowCount(frequency, density, snr);
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
        // DESIGN NOTE: we can't c_maxIdfX10Value directly to min because min
        // takes a reference and the compiler has already turned it into a
        // constant, which we can't take a reference to.
        auto local = Term::c_maxIdfX10Value;
        Term::IdfX10 idf = std::min(term.GetIdfSum(), local);
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
                // TODO: consider checking for overflow?
                int k = Term::ComputeRowCount(frequency, density, snr);
                configuration.push_front(RowConfiguration::Entry(0, 2, false));
                if (k > 2)
                {
                    Rank rank = 3;
                    double frequencyAtRank = Term::FrequencyAtRank(frequency, rank);
                    if (frequencyAtRank >= density)
                    {
                        configuration.push_front(RowConfiguration::Entry(rank, 1, true));
                    }
                    else
                    {
                        configuration.push_front(RowConfiguration::Entry(rank, k - 2, false));
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
        // DESIGN NOTE: we can't c_maxIdfX10Value directly to min because min
        // takes a reference and the compiler has already turned it into a
        // constant, which we can't take a reference to.
        auto local = Term::c_maxIdfX10Value;
        Term::IdfX10 idf = std::min(term.GetIdfSum(), local);
        return m_configurations[idf];
    }


    //*************************************************************************
    //
    // TreatmentPrivateSharedRank0ToN
    //
    // Two rank 0 rows followed by rows of increasing rank until the bit density
    // is > .5 or we run out of rows. Due to limitatons in other BitFunnel code,
    // we also top out at rank 6.
    //
    //*************************************************************************
    TreatmentPrivateSharedRank0ToN::TreatmentPrivateSharedRank0ToN(double density, double snr)
    {
        // TODO: what should maxDensity be? Note that this is different from the
        // density liimt that's passed in.
        const double maxDensity = 0.5;
        // Fill up vector of RowConfigurations. GetTreatment() will use the
        // IdfSum() value of the Term as an index into this vector.
        //
        // TODO: we should make sure we get "enough" rows if we end up with a
        // high rank private row.
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
                // TODO: fix other limitations so this can be higher than 6?
                const Rank maxRank = (std::min)(Term::ComputeMaxRank(frequency, maxDensity), static_cast<Rank>(6u));

                int numRows = Term::ComputeRowCount(frequency, density, snr);
                configuration.push_front(RowConfiguration::Entry(0, 2, false));
                numRows -= 2;
                Rank rank = 1;
                while (rank < maxRank && numRows > 0)
                {
                    double frequencyAtRank = Term::FrequencyAtRank(frequency, rank);
                    if (frequencyAtRank >= density)
                    {
                        configuration.push_front(RowConfiguration::Entry(rank,
                                                                         1,
                                                                         true));
                    }
                    else
                    {
                        configuration.push_front(RowConfiguration::Entry(rank,
                                                                         1,
                                                                         false));
                    }
                    ++rank;
                    --numRows;
                }
                if (numRows > 0)
                {
                    double frequencyAtRank = Term::FrequencyAtRank(frequency, rank);
                    if (frequencyAtRank >= density)
                    {
                        configuration.push_front(RowConfiguration::Entry(rank,
                                                                         1,
                                                                         true));
                    }
                    else
                    {
                        configuration.push_front(RowConfiguration::Entry(rank,
                                                                         numRows,
                                                                         false));
                    }
                }
            }

            m_configurations.push_back(configuration);

            //std::cout << idf / 10.0 << ": ";
            //m_configurations.back().Write(std::cout);
            //std::cout << std::endl;
        }
    }


    RowConfiguration TreatmentPrivateSharedRank0ToN::GetTreatment(Term term) const
    {
        // DESIGN NOTE: we can't c_maxIdfX10Value directly to min because min
        // takes a reference and the compiler has already turned it into a
        // constant, which we can't take a reference to.
        auto local = Term::c_maxIdfX10Value;
        Term::IdfX10 idf = std::min(term.GetIdfSum(), local);
        return m_configurations[idf];
    }
}
