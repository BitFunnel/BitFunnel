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

#include <iostream>
#include <limits>

#include "BitFunnel/Exceptions.h"
#include "BitFunnel/IDiagnosticStream.h"
#include "BitFunnel/Index/DocumentHandle.h"
#include "BitFunnel/Index/Factories.h"
#include "BitFunnel/Plan/QueryInstrumentation.h"
#include "ByteCodeInterpreter.h"
#include "CacheLineRecorder.h"
#include "LoggerInterfaces/Check.h"
#include "ResultsBuffer.h"


namespace BitFunnel
{
/*
TODO

Review zero flag.
Address TODO comments.

Decide on type of Slices
  void const * or uint64_t const *
  If uint64_t, must ensure 8-byte alignment of buffer and rows.
  This must be commented. In other words, rows are not only aligned
  for performance - they are also aligned to support pointer arithmatic.

*/

    //*************************************************************************
    //
    // ByteCodeInterpreter
    //
    //*************************************************************************
    ByteCodeInterpreter::ByteCodeInterpreter(
        ByteCodeGenerator const & code,
        ResultsBuffer & resultsBuffer,
        size_t sliceCount,
        void * const * sliceBuffers,
        size_t iterationsPerSlice,
        ptrdiff_t const * rowOffsets,
        IDiagnosticStream * diagnosticStream,
        QueryInstrumentation & instrumentation,
        CacheLineRecorder * cacheLineRecorder)
      : m_code(code.GetCode()),
        m_jumpTable(code.GetJumpTable()),
        m_resultsBuffer(resultsBuffer),
        m_sliceCount(sliceCount),
        m_sliceBuffers(sliceBuffers),
        m_iterationsPerSlice(iterationsPerSlice),
        m_rowOffsets(rowOffsets),
        m_dedupe(),
        m_diagnosticStream(diagnosticStream),
        m_instrumentation(instrumentation),
        m_cacheLineRecorder(cacheLineRecorder)
    {
    }


    bool ByteCodeInterpreter::Run()
    {
        for (size_t i = 0; i < m_sliceCount; ++i)
        {
            bool terminate = ProcessOneSlice(i);
            if (terminate)
            {
                return true;
            }
        }

        // false ==> ran to completion.
        return false;
    }


    bool ByteCodeInterpreter::ProcessOneSlice(size_t slice)
    {
        auto sliceBuffer = m_sliceBuffers[slice];

        if (m_cacheLineRecorder != nullptr)
        {
            m_cacheLineRecorder->Reset();
            m_cacheLineRecorder->SetBase(sliceBuffer); 
        }

        bool terminate = false;

        for (size_t i = 0; i < m_iterationsPerSlice; ++i)
        {
            terminate = RunOneIteration(sliceBuffer, i);
            if (terminate)
            {
                break;
            }
        }

        if (m_cacheLineRecorder != nullptr)
        {
            m_instrumentation.IncrementCacheLineCount(
                m_cacheLineRecorder->GetCacheLinesAccessed());
        }

        // false ==> ran to completion.
        return false;
    }


    bool ByteCodeInterpreter::RunOneIteration(
        void const * voidSliceBuffer,
        size_t iteration)
    {
        char const * sliceBuffer =
            reinterpret_cast<char const *>(voidSliceBuffer);

        bool calledAddResult = false;
        m_ip = m_code.data();
        m_offset = iteration;

        if (m_diagnosticStream != nullptr && 
            m_diagnosticStream->IsEnabled("bytecode/opcode"))
        {
            std::ostream& out = m_diagnosticStream->GetStream();
            out << "--------------------" << std::endl;
            out << "ByteCode RunOneIteration:" << std::endl;
        }

        while (m_ip->GetOpcode() != Opcode::End)
        {
            const Opcode opcode = m_ip->GetOpcode();
            const unsigned row = m_ip->GetRow();
            const unsigned delta = m_ip->GetDelta();
            const bool inverted = m_ip->IsInverted();

            if (m_diagnosticStream != nullptr &&
                m_diagnosticStream->IsEnabled("bytecode/opcode"))
            {
                std::ostream& out = m_diagnosticStream->GetStream();
                out << "IP: " << std::hex << m_ip << std::dec << std::endl
                    << "Opcode: " << opcode << std::endl
                    << "Offset: " << m_offset << std::endl
                    << "Row: " << row << std::endl
                    << "RowOffset: " << std::hex << m_rowOffsets[row] << std::dec << std::endl;
            }
            switch (opcode)
            {
            case Opcode::AndRow:
                {
                    m_instrumentation.IncrementQuadwordCount();
                    uint64_t const * rowPtr =
                        reinterpret_cast<uint64_t const *>(
                            sliceBuffer + m_rowOffsets[row]);

                    auto ptr = rowPtr + (m_offset >> delta);
                    if (m_cacheLineRecorder != nullptr)
                    {
                        m_cacheLineRecorder->RecordAccess(ptr);
                    }

                    uint64_t value = *ptr;
                    m_accumulator &= (inverted ? ~value : value);
                    m_zeroFlag = (m_accumulator == 0);
                    m_ip++;

                    if (m_diagnosticStream != nullptr &&
                        m_diagnosticStream->IsEnabled("bytecode/loadrow"))
                    {
                        std::ostream& out = m_diagnosticStream->GetStream();
                        out << "AndRow: " << std::hex << m_accumulator
                            << std::dec << std::endl;
                    }
                }
                break;
            case Opcode::LoadRow:
                {
                    m_instrumentation.IncrementQuadwordCount();
                    uint64_t const * rowPtr =
                        reinterpret_cast<uint64_t const *>(
                            sliceBuffer + m_rowOffsets[row]);

                    auto ptr = rowPtr + (m_offset >> delta);
                    if (m_cacheLineRecorder != nullptr)
                    {
                        m_cacheLineRecorder->RecordAccess(ptr);
                    }

                    auto value = *ptr;
                    m_accumulator = (inverted ? ~value : value);
                    m_zeroFlag = (m_accumulator == 0);
                    m_ip++;

                    if (m_diagnosticStream != nullptr &&
                        m_diagnosticStream->IsEnabled("bytecode/loadrow"))
                    {
                        std::ostream& out = m_diagnosticStream->GetStream();
                        out << "LoadRow: " << std::hex << m_accumulator
                            << std::dec << std::endl;
                    }
                }
                break;
            case Opcode::LeftShiftOffset:
                m_offset <<= row;
                m_ip++;
                break;
            case Opcode::RightShiftOffset:
                m_offset >>= row;
                m_ip++;
                break;
            case Opcode::IncrementOffset:
                m_offset++;
                m_ip++;
                break;
            case Opcode::Push:
                m_valueStack.push_back(m_accumulator);
                m_ip++;
                break;
            case Opcode::Pop:
                m_accumulator = m_valueStack.back();
                m_valueStack.pop_back();
                m_ip++;
                break;
            case Opcode::AndStack:
                {
                    m_accumulator &= m_valueStack.back();
                    m_valueStack.pop_back();
                    m_ip++;
                }
                break;
            case Opcode::Constant:
                throw NotImplemented("Constant opcode not implemented.");
            case Opcode::Not:
                m_accumulator = !m_accumulator;
                m_ip++;
                break;
            case Opcode::OrStack:
                {
                    m_accumulator |= m_valueStack.back();
                    m_valueStack.pop_back();
                    m_ip++;
                }
                break;
            case Opcode::UpdateFlags:
                m_zeroFlag = (m_valueStack.back() == 0);
                m_ip++;
                break;
            case Opcode::Report:
                // TODO: Combine accumulator with value stack.
                if (m_accumulator != 0)
                {
                    AddResult(m_accumulator, m_offset);
                    calledAddResult = true;
                }
                m_ip++;
                break;
            case Opcode::Call:
                m_callStack.push_back(m_ip + 1);
                m_ip = m_jumpTable[row];
                break;
            case Opcode::Jmp:
                m_ip = m_jumpTable[row];
                break;
            case Opcode::Jnz:
                if (m_accumulator != 0ull)
                {
                    m_ip = m_jumpTable[row];
                }
                else
                {
                    m_ip++;
                }
                break;
            case Opcode::Jz:
                if (m_accumulator == 0ull)
                {
                    m_ip = m_jumpTable[row];
                }
                else
                {
                    m_ip++;
                }
                break;
            case Opcode::Return:
                m_ip = m_callStack.back();
                m_callStack.pop_back();
                break;
            default:
                RecoverableError error("ByteCodeInterpreter:: bad opcode.");
                throw error;
            }  // switch
        }  // while


        bool terminate = false;
        if (calledAddResult)
        {
            terminate = FinishIteration(sliceBuffer);
        }

        return terminate;
    }
    
    
    void ByteCodeInterpreter::AddResult(uint64_t accumulator,
                                        size_t offset)
    {
        // Set bit indicating that we're storing an accululator at offset.
        m_dedupe[0] |= (1ull << offset);

        // Or in the accumulator.
        m_dedupe[offset + 1] |= accumulator;
    }


    static uint64_t bsf(uint64_t value)
    {
#ifdef _MSC_VER
        unsigned long index;
        _BitScanForward64(&index, value);
        return index;
#else
        // DESIGN NOTE: this is undefined if the input operand i 0. We currently
        // only call lzcnt in one place, where we've prevously gauranteed that
        // the input isn't 0. This is not safe to use in the general case.
        return static_cast<uint64_t>(__builtin_ctzll(value));
#endif
    }


    bool ByteCodeInterpreter::FinishIteration(void const * sliceBuffer)
    {
        uint64_t map = m_dedupe[0];
        while (map != 0)
        {
            size_t offset = bsf(map);

            uint64_t accumulator = m_dedupe[offset + 1];

            while (accumulator != 0)
            {
                size_t bitPos = bsf(accumulator);

                DocIndex docIndex = offset * c_bitsPerQuadword + bitPos;

                // TODO: find a better way to get the Slice pointer.
                Slice* slice = *reinterpret_cast<Slice**>(const_cast<void*>(sliceBuffer));
                m_resultsBuffer.push_back(slice, docIndex);

                // Clear the lowest bit set in the accumulator.
                accumulator &= (accumulator - 1);
            }
            m_dedupe[offset + 1] = 0;

            // Clear the lowest bit set in the map.
            map &= (map - 1);
        }
        m_dedupe[0] = 0;

        // TODO: don't always return false.
        return false;
    }


    //*************************************************************************
    //
    // ByteCodeGenerator
    //
    //*************************************************************************
    ByteCodeGenerator::ByteCodeGenerator()
        : m_sealed(false)
    {
    }


    void ByteCodeGenerator::Seal()
    {
        EnsureSealed(false);

        // Mark the end of the generated code.
        m_code.emplace_back(ByteCodeInterpreter::Opcode::End);

        // Verify that all labels have been placed.
        for (size_t i = 0; i < m_jumpTable.size(); ++i)
        {
            CHECK_NE(m_jumpTable[i], nullptr)
                << "Label " << i << " has not been placed.";
        }

        // Lookup ip values for each jump offset.
        for (size_t offset : m_jumpOffsets)
        {
            m_jumpTable.push_back(&m_code[0] + offset);
        }

        m_sealed = true;
    }


    std::vector<ByteCodeInterpreter::Instruction> const &
        ByteCodeGenerator::GetCode() const
    {
        EnsureSealed(true);
        return m_code;
    }


    std::vector<ByteCodeInterpreter::Instruction const *> const &
        ByteCodeGenerator::GetJumpTable() const
    {
        EnsureSealed(true);
        return m_jumpTable;
    }


    void ByteCodeGenerator::AndRow(size_t row, bool inverted, size_t rankDelta)
    {
        EnsureSealed(false);
        m_code.emplace_back(
            ByteCodeInterpreter::Opcode::AndRow, row, rankDelta, inverted);
    }


    void ByteCodeGenerator::LoadRow(size_t row, bool inverted, size_t rankDelta)
    {
        EnsureSealed(false);
        m_code.emplace_back(
            ByteCodeInterpreter::Opcode::LoadRow, row, rankDelta, inverted);
    }


    void ByteCodeGenerator::LeftShiftOffset(size_t shift)
    {
        EnsureSealed(false);
        m_code.emplace_back(
            ByteCodeInterpreter::Opcode::LeftShiftOffset, shift);
    }


    void ByteCodeGenerator::RightShiftOffset(size_t shift)
    {
        EnsureSealed(false);
        m_code.emplace_back(
            ByteCodeInterpreter::Opcode::RightShiftOffset, shift);
    }


    void ByteCodeGenerator::IncrementOffset()
    {
        EnsureSealed(false);
        m_code.emplace_back(
            ByteCodeInterpreter::Opcode::IncrementOffset);
    }


    void ByteCodeGenerator::Push()
    {
        EnsureSealed(false);
        m_code.emplace_back(
            ByteCodeInterpreter::Opcode::Push);
    }


    void ByteCodeGenerator::Pop()
    {
        EnsureSealed(false);
        m_code.emplace_back(
            ByteCodeInterpreter::Opcode::Pop);
    }


    void ByteCodeGenerator::AndStack()
    {
        EnsureSealed(false);
        m_code.emplace_back(
            ByteCodeInterpreter::Opcode::AndStack);
    }


    void ByteCodeGenerator::Constant(int /*value*/)
    {
        EnsureSealed(false);
        throw NotImplemented("Constant opcode not implemented.");
    }


    void ByteCodeGenerator::Not()
    {
        EnsureSealed(false);
        m_code.emplace_back(
            ByteCodeInterpreter::Opcode::Not);
    }


    void ByteCodeGenerator::OrStack()
    {
        EnsureSealed(false);
        m_code.emplace_back(
            ByteCodeInterpreter::Opcode::OrStack);
    }


    void ByteCodeGenerator::UpdateFlags()
    {
        EnsureSealed(false);
        m_code.emplace_back(
            ByteCodeInterpreter::Opcode::UpdateFlags);
    }


    void ByteCodeGenerator::Report()
    {
        EnsureSealed(false);
        m_code.emplace_back(
            ByteCodeInterpreter::Opcode::Report);
    }


    ICodeGenerator::Label ByteCodeGenerator::AllocateLabel()
    {
        EnsureSealed(false);
        Label label = static_cast<Label>(m_jumpOffsets.size());

        // Use std::numeric_limits<size_t>::max() to mark this offset
        // as allocated, but uninitialized.
        m_jumpOffsets.push_back(std::numeric_limits<size_t>::max());
        return label;
    }


    void ByteCodeGenerator::PlaceLabel(Label label)
    {
        EnsureSealed(false);
        CHECK_EQ(m_jumpOffsets.at(label), std::numeric_limits<size_t>::max())
            << "Label " << label << " has already been placed.";

        m_jumpOffsets.at(label) = m_code.size();
    }


    void ByteCodeGenerator::Call(Label label)
    {
        EnsureSealed(false);
        CHECK_LT(label, m_jumpOffsets.size())
            << "Call to unknown label " << label;

        m_code.emplace_back(
            ByteCodeInterpreter::Opcode::Call, label);
    }


    void ByteCodeGenerator::Jmp(Label label)
    {
        EnsureSealed(false);
        CHECK_LT(label, m_jumpOffsets.size())
            << "Jmp to unknown label " << label;

        m_code.emplace_back(
            ByteCodeInterpreter::Opcode::Jmp, label);
    }


    void ByteCodeGenerator::Jnz(Label label)
    {
        EnsureSealed(false);
        CHECK_LT(label, m_jumpOffsets.size())
            << "Jnz to unknown label " << label;

        m_code.emplace_back(
            ByteCodeInterpreter::Opcode::Jnz, label);
    }


    void ByteCodeGenerator::Jz(Label label)
    {
        EnsureSealed(false);
        CHECK_LT(label, m_jumpOffsets.size())
            << "Jz to unknown label " << label;

        m_code.emplace_back(
            ByteCodeInterpreter::Opcode::Jz, label);
    }


    void ByteCodeGenerator::Return()
    {
        EnsureSealed(false);
        m_code.emplace_back(
            ByteCodeInterpreter::Opcode::Return);
    }


    void ByteCodeGenerator::EnsureSealed(bool sealed) const
    {
        CHECK_EQ(sealed, m_sealed)
            << (sealed ? "class must be sealed." :
                         "class already sealed.");
    }
}
