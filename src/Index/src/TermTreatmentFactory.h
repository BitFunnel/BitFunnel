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

#include <string>   // std::string embedded.
#include <vector>   // std::vector embedded.

#include "BitFunnel/Index/ITermTreatmentFactory.h"  // Base class.
#include "LoggerInterfaces/Check.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // TermTreatmentFactory
    //
    //*************************************************************************
    class TermTreatmentFactory : public ITermTreatmentFactory
    {
    public:
        TermTreatmentFactory();

        virtual std::vector<std::string> const &
            GetTreatmentNames() const override;

        virtual std::vector<std::string> const &
            GetTreatmentDescriptions() const override;

        virtual std::unique_ptr<ITermTreatment>
            CreateTreatment(char const * name,
                            double density,
                            double snr) const override;

    private:
        typedef std::unique_ptr<ITermTreatment>(*Creator)(
            double density,
            double snr);

        template <class TREATMENT>
        static std::unique_ptr<ITermTreatment> Create(double density,
                                                      double snr)
        {
            return std::unique_ptr<ITermTreatment>
                (new TREATMENT(density, snr));
        }

        template <class TREATMENT>
        void AddTreatment()
        {
            auto name = TREATMENT::GetName();
            for (auto & treatment : m_names)
            {
                CHECK_NE(treatment, name)
                    << "Duplicate ITermTreatment name.";
            }

            m_names.push_back(name);
            m_descriptions.push_back(TREATMENT::GetDescription());
            m_creators.push_back(Create<TREATMENT>);
        }

        std::vector<std::string> m_names;
        std::vector<std::string> m_descriptions;
        std::vector<Creator> m_creators;
    };
}
