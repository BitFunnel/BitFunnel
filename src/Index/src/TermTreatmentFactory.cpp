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

#include "BitFunnel/Index/Factories.h"
#include "LoggerInterfaces/Check.h"
#include "TermTreatmentFactory.h"
#include "TreatmentClassicBitsliced.h"
#include "TreatmentOptimal.h"
#include "TreatmentPrivateRank0.h"
#include "TreatmentPrivateSharedRank0.h"
#include "TreatmentPrivateSharedRank0And3.h"
#include "TreatmentPrivateSharedRank0ToN.cpp"


namespace BitFunnel
{
    std::unique_ptr<ITermTreatmentFactory> Factories::CreateTreatmentFactory()
    {
        return std::unique_ptr<ITermTreatmentFactory>(new TermTreatmentFactory());
    }


    //*************************************************************************
    //
    // TermTreatmentFactory
    //
    //*************************************************************************
    TermTreatmentFactory::TermTreatmentFactory()
    {
        AddTreatment<TreatmentPrivateRank0>();
        AddTreatment<TreatmentPrivateSharedRank0>();
        AddTreatment<TreatmentPrivateSharedRank0And3>();
        AddTreatment<TreatmentPrivateSharedRank0ToN>();
        AddTreatment<TreatmentOptimal>();
        AddTreatment<TreatmentClassicBitsliced>();
    }


    std::vector<std::string> const &
        TermTreatmentFactory::GetTreatmentNames() const
    {
        return m_names;
    }


    std::vector<std::string> const &
        TermTreatmentFactory::GetTreatmentDescriptions() const
    {
        return m_descriptions;
    }


    std::unique_ptr<ITermTreatment>
        TermTreatmentFactory::CreateTreatment(char const * name,
                                              double density,
                                              double snr) const
    {
        size_t i = 0;
        for (i = 0; i < m_names.size(); ++i)
        {
            if (m_names[i] == name)
            {
                return m_creators[i](density, snr);
            }
        }

        CHECK_FAIL
            << "Unknown ITermTreatment " << name;
    }
}
