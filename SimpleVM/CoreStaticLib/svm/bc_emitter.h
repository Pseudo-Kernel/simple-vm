
#pragma once

#include "base.h"

namespace VM_NAMESPACE
{
    class VMBytecodeEmitter
    {
    public:
        VMBytecodeEmitter() noexcept;

        VMBytecodeEmitter& BeginEmit() noexcept;
        VMBytecodeEmitter& Emit(Opcode::T Opcode) noexcept;
        VMBytecodeEmitter& Emit(Opcode::T Opcode, uint8_t Immediate) noexcept;
        VMBytecodeEmitter& Emit(Opcode::T Opcode, Operand Operand) noexcept;
        bool EndEmit(unsigned char* Buffer, size_t Size, size_t* SizeRequired) noexcept;

    private:
        std::vector<VMInstruction> OpList_;
        bool Begin_;
    };
}

