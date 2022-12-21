
#include "vmbase.h"
#include "bc_emitter.h"

namespace VM_NAMESPACE
{
    VMBytecodeEmitter::VMBytecodeEmitter() noexcept : Begin_()
    {
    }

    VMBytecodeEmitter& VMBytecodeEmitter::BeginEmit() noexcept
    {
        OpList_.clear();
        Begin_ = true;

        return *this;
    }

    VMBytecodeEmitter& VMBytecodeEmitter::Emit(Opcode::T Opcode) noexcept
    {
        OpList_.push_back(VMInstruction::Create(Opcode));
        return *this;
    }

    VMBytecodeEmitter& VMBytecodeEmitter::Emit(Opcode::T Opcode, uint8_t Immediate) noexcept
    {
        OpList_.push_back(VMInstruction::Create(Opcode, Immediate));
        return *this;
    }

    VMBytecodeEmitter& VMBytecodeEmitter::Emit(Opcode::T Opcode, Operand Operand) noexcept
    {
        VMInstruction Op;

        switch (Operand.Type)
        {
        case OperandType::T::Imm8:
            Op = VMInstruction::Create(Opcode, static_cast<uint8_t>(Operand.Value));
            break;
        case OperandType::T::Imm16:
            Op = VMInstruction::Create(Opcode, static_cast<uint16_t>(Operand.Value));
            break;
        case OperandType::T::Imm32:
            Op = VMInstruction::Create(Opcode, static_cast<uint32_t>(Operand.Value));
            break;
        case OperandType::T::Imm64:
            Op = VMInstruction::Create(Opcode, static_cast<uint64_t>(Operand.Value));
            break;
        default:
            DASSERT(false);
        }

        OpList_.push_back(Op);

        return *this;
    }

    bool VMBytecodeEmitter::EndEmit(unsigned char* Buffer, size_t Size, size_t* SizeRequired) noexcept
    {
        if (!Begin_)
            return 0;

        size_t SizeExpected = 0;

        for (auto& Op : OpList_)
        {
            size_t OpSize = 0;
            Op.ToBytes(nullptr, 0, &OpSize);
            if (!OpSize)
                return false;
            SizeExpected += OpSize;
        }

        if (SizeRequired)
            *SizeRequired = SizeExpected;

        if (SizeExpected > Size)
            return false;

        unsigned char* p = Buffer;
        size_t SizeEmit = 0;

        for (auto& Op : OpList_)
        {
            size_t OpSize = 0;
            DASSERT(Op.ToBytes(p, Size - SizeEmit, &OpSize));
            SizeEmit += OpSize;
            p += OpSize;
        }

        return true;
    }
}

