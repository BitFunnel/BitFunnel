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

#include <cstdint>
#include <vector>

#include "BitFunnel/BitFunnelTypes.h"       // Rank parameter.
#include "BitFunnel/Plan/ICodeGenerator.h"  // Base class.


namespace BitFunnel
{
    class ByteCodeGenerator;

    class ByteCodeInterpreter
    {
    public:
        class Instruction;

        ByteCodeInterpreter(ByteCodeGenerator & code,
                            uint64_t const * const * rows);

        void Run(size_t iterationCount);

        enum class Opcode
        {
            AndRow,
            LoadRow,
            LeftShiftOffset,
            RightShiftOffset,
            IncrementOffset,
            Push,
            Pop,
            AndStack,
            Constant,
            Not,
            OrStack,
            UpdateFlags,
            Report,
            Call,
            Jmp,
            Jnz,
            Jz,
            Return,
            End,
            Last
        };

        static const uint32_t c_opCodeBits = 5;
        static_assert(static_cast<unsigned>(Opcode::Last) < (1ul << c_opCodeBits),
                      "Instruction::m_opcode does not have enough bits.");

        class Instruction
        {
        public:
            Instruction(Opcode opcode, size_t row = 0ul, size_t delta = 0ul, bool inverted = false)
              : m_opcode(static_cast<uint32_t>(opcode)),
                m_row(static_cast<uint32_t>(row)),
                m_delta(static_cast<uint32_t>(delta)),
                m_inverted(inverted ? 1 : 0)
            {
                // TODO: Bounds check parameters.
            }

            Opcode GetOpcode() const
            {
                return static_cast<Opcode>(m_opcode);
            }

            unsigned GetRow() const
            {
                return m_row;
            }

            unsigned GetDelta() const
            {
                return m_delta;
            }

            bool IsInverted() const
            {
                return m_inverted == 1ul;
            }

        private:
            uint32_t m_opcode : c_opCodeBits;
            uint32_t m_row : 10;
            uint32_t m_delta : 4;
            uint32_t m_inverted : 1;
        };

    private:
        void RunOneIteration(size_t iteration);

        std::vector<Instruction> const & m_code;
        std::vector<Instruction const *> const & m_jumpTable;

        uint64_t const * const * m_rows;

        size_t m_iteration;
        size_t m_offset;
        Instruction const * m_ip;
        uint64_t m_accumulator;

        std::vector<Instruction const *> m_callStack;
        std::vector<uint64_t> m_valueStack;
        bool m_zeroFlag;
    };


    class ByteCodeGenerator : public ICodeGenerator
    {
    public:
        std::vector<ByteCodeInterpreter::Instruction> const & GetCode() const;
        std::vector<ByteCodeInterpreter::Instruction const *> const & GetJumpTable() const;

        //
        // ICodeGenerator methods
        //
        // RankDown compiler primitives
        virtual void AndRow(size_t id, bool inverted, size_t rankDelta) override;
        virtual void LoadRow(size_t id, bool inverted, size_t rankDelta) override;

        virtual void LeftShiftOffset(size_t shift) override;
        virtual void RightShiftOffset(size_t shift) override;
        virtual void IncrementOffset() override;

        virtual void Push() override;
        virtual void Pop() override;

        // Stack machine primitives
        virtual void AndStack() override;
        virtual void Constant(int value) override;
        virtual void Not() override;
        virtual void OrStack() override;
        virtual void UpdateFlags() override;

        virtual void Report() override;

        // Constrol flow primitives.
        virtual ICodeGenerator::Label AllocateLabel() override;
        virtual void PlaceLabel(Label label) override;
        virtual void Call(Label label) override;
        virtual void Jmp(Label label) override;
        virtual void Jnz(Label label) override;
        virtual void Jz(Label label) override;
        virtual void Return() override;

        // TODO: Add this to ICodeGenerator.
        void End();

    private:
        std::vector<ByteCodeInterpreter::Instruction> m_code;
        std::vector<ByteCodeInterpreter::Instruction const *> m_jumpTable;
    };
}
