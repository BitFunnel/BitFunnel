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


#include "BitFunnel/Term.h"
#include "TreatmentPrivateRank0.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // TreatmentPrivateRank0
    //
    // All terms get the same treatment - a single, private, rank 0 row.
    //
    //*************************************************************************
    TreatmentPrivateRank0::TreatmentPrivateRank0(double /*density*/,
                                                 double /*snr*/)
    {
        // Same configuration for all terms - one private rank 0 row.
        m_configuration.push_front(RowConfiguration::Entry(0, 1));

        //std::cout << "Single configuration: ";
        //m_configuration.Write(std::cout);
        //std::cout << std::endl;
    }


    RowConfiguration TreatmentPrivateRank0::GetTreatment(Term::IdfX10 /*term*/) const
    {
        return m_configuration;
    }
}
