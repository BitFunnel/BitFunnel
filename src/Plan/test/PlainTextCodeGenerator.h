#pragma once

#include <ostream>                          // std::ostream parameter.

#include "BitFunnel/NonCopyable.h"          // Inherits from NonCopyable.
#include "BitFunnel/Plan/ICodeGenerator.h"  // Inherits from ICodeGenerator.


namespace BitFunnel
{
    class PlainTextCodeGenerator : public ICodeGenerator, NonCopyable
    {
    public:
        PlainTextCodeGenerator(std::ostream& output);

        void AndRow(size_t id, bool inverted, size_t rankDelta);
        void LoadRow(size_t id, bool inverted, size_t rankDelta);

        void LeftShiftOffset(size_t shift);
        void RightShiftOffset(size_t shift);
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
        void EmitSizeTArg(char const* name, size_t arg);
        void EmitRowArg(char const* name,
                        size_t id,
                        bool inverted,
                        size_t rankDelta);
        void Indent();

        std::ostream& m_output;
        Label m_label;
    };
}
