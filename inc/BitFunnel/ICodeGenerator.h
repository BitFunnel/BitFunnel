#pragma once


namespace BitFunnel
{
    //*************************************************************************
    //
    // ICodeGenerator is an abstract base class or interface for classes that
    // translate BitFunnel matching and scoring primitives into other forms
    // such as X64 machine code or a textual representation for unit tests.
    //
    //*************************************************************************
    class ICodeGenerator
    {
    public:
        typedef unsigned Label;

        virtual ~ICodeGenerator() {};

        // RankDown compiler primitives
        virtual void AndRow(unsigned id, bool inverted, unsigned rankDelta) = 0;
        virtual void LoadRow(unsigned id, bool inverted, unsigned rankDelta) = 0;

        virtual void LeftShiftOffset(unsigned shift) = 0;
        virtual void RightShiftOffset(unsigned shift) = 0;
        virtual void IncrementOffset() = 0;

        virtual void Push() = 0;
        virtual void Pop() = 0;

        // Stack machine primitives
        virtual void AndStack() = 0;
        virtual void Constant(int value) = 0;
        virtual void Not() = 0;
        virtual void OrStack() = 0;
        virtual void UpdateFlags() = 0;

        virtual void Report() = 0;

        // Constrol flow primitives.
        virtual Label AllocateLabel() = 0;
        virtual void PlaceLabel(Label label) = 0;
        virtual void Call(Label label) = 0;
        virtual void Jmp(Label label) = 0;
        virtual void Jnz(Label label) = 0;
        virtual void Jz(Label label) = 0;
        virtual void Return() = 0;
    };
}
