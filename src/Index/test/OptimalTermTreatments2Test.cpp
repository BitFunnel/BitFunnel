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


#include "gtest/gtest.h"

#include <array>

#include "OptimalTermTreatments.h"
#include "OptimalTermTreatments2.h"
//#include "TermTreatments.h"


namespace BitFunnel
{
    TEST(OptimalTermTreatments2Test, Parity)
    {
        double density = 0.15;
        double snr = 10.0;
        int variant = 0;

//        std::array<size_t, 4> configurations{ { 1, 3, 123, 12023 } };

        TreatmentOptimal2 treatments(density, snr, variant);

        //double signal = 0.001;
        //auto config2 = treatments.FindBestTreatment(density, signal, snr, variant);
        //auto config1 = FindBestTreatment(density, signal, snr, variant);
        //ASSERT_EQ(config1, config2);


        for (Term::IdfX10 idf = 0; idf <= Term::c_maxIdfX10Value; ++idf)
        {
            double signal = Term::IdfX10ToFrequency(idf);

            // Algorithm only valid for terms with signal <= density.
            // TODO: Understand why signal >= density seems to lead to term
            // treatments with higher rank rows.
            if (signal < density)
            {
                std::cout
                    << std::setprecision(2) << "idf " << 0.1 * idf
                    << ": " << std::setprecision(6);

                // auto config2 = 
                treatments.FindBestTreatment(density, signal, snr, variant);
                //auto config1 = FindBestTreatment(density, signal, snr, variant);
                //ASSERT_EQ(config1, config2);

                //auto config = FindBestTreatment(density, signal, snr, variant);
                //m_configurations.push_back(RowConfigurationFromSizeT(config));
            }
            //else
            //{
            //    std::cout
            //        << "idf " << std::setprecision(2) << 0.1 * idf
            //        << ": " << std::setprecision(6) << signal
            //        << " ==> private row" << std::endl;
            //    RowConfiguration configuration;
            //    configuration.push_front(RowConfiguration::Entry(0, 1));
            //    m_configurations.push_back(configuration);
            //}
        }

        //for (auto configuration : configurations)
        //{
        //    double signal = 0.001;

        //    auto metrics1 = Analyze(configuration, density, signal, true).second;
        //    auto metrics2 = treatments.Analyze(configuration, density, signal, true);
        //    ASSERT_EQ(metrics1, metrics2);
        //}
    }
}
