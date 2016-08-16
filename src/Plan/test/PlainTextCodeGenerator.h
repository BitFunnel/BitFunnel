#pragma once

#include <ostream>                        // std::ostream used as parameter.

#include "BitFunnel/ICodeGenerator.h"     // Inherits from ICodeGenerator.
#include "BitFunnel/NonCopyable.h"        // Inherits from NonCopyable.


namespace BitFunnel
{
    class PlainTextCodeGenerator : public ICodeGenerator, NonCopyable
    {
    public:
        PlainTextCodeGenerator(std::ostream& output);

        void AndRow(unsigned id, bool inverted, unsigned rankDelta);
        void LoadRow(unsigned id, bool inverted, unsigned rankDelta);

        void LeftShiftOffset(unsigned shift);
        void RightShiftOffset(unsigned shift);
        void IncrementOffset();

        void Push();
        void Pop();

        void AddStack();
        void AndStack();
        void Constant(int value);
        void MaxStack();
        void Not();
        void OrStack();
        void UpdateFlags();

        void Report();

        Label AllocateLabel();
        void PlaceLabel(Label label);
        void Call(Label label);
        void Jmp(Label label);
        void Jnz(Label label);
        void Jz(Label label);
        void Return();

    private:
        void EmitZeroArg(char const* name);
        void EmitSignedArg(char const* name, int arg);
        void EmitUnsignedArg(char const* name, unsigned arg);
        void EmitRowArg(char const* name,
                        unsigned id,
                        bool inverted,
                        unsigned rankDelta);
        void Indent();

        std::ostream& m_output;
        Label m_label;
    };
}
