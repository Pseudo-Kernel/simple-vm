
#include "vmbase.h"
#include "vminst.h"

namespace VM_NAMESPACE
{
    const InstructionInfo VMInstruction::InstructionList[]
    {
        #include "inst_table.h" // instruction table
    };

    const size_t VMInstruction::InstructionMaximumSize = 0x10; // Prefix(1) + Opcode(2) + Imm64(8) + Reserved(5)


    VMInstruction::VMInstruction() :
        Opcode_(), Valid_(false)
    {
    }

    VMInstruction::VMInstruction(Opcode::T Opcode, unsigned char* ImmediateBytes, size_t ImmediateSize) :
        Opcode_(), Immediate_{}, ImmediateSize_(), OperandCount_(), Valid_(true)
    {
        SetOpcode(Opcode);
        SetOperand(ImmediateBytes, ImmediateSize);
    }

    Opcode::T VMInstruction::Opcode() const
    {
        return static_cast<Opcode::T>(Opcode_);
    }

    size_t VMInstruction::OpcodeSize() const
    {
        return OpcodeSize_;
    }

    int VMInstruction::OperandCount() const
    {
        return OperandCount_;
    }

    uint8_t VMInstruction::Operand(int Index, uint8_t* Buffer, uint8_t Size)
    {
        DASSERT(Index == 0);

        if (ImmediateSize_)
        {
            auto CopySize = Size;
            if (CopySize > ImmediateSize_)
                CopySize = ImmediateSize_;

            memcpy(Buffer, Immediate_, CopySize);
        }

        return ImmediateSize_;
    }

    uint8_t VMInstruction::OperandSize(int Index) const
    {
        DASSERT(Index == 0);
        return ImmediateSize_;
    }

    bool VMInstruction::Valid() const
    {
        return Valid_;
    }

    bool VMInstruction::SetOpcode(Opcode::T Opcode)
    {
        return SetOpcode(static_cast<uint16_t>(Opcode));
    }

    bool VMInstruction::SetOpcode(uint16_t Opcode)
    {
        if (Opcode > 0x3fff)
        {
            OpcodeSize_ = 0;
            return false;
        }
        else if (Opcode > 0x7f)
        {
            Opcode_ = Opcode;
            OpcodeSize_ = 2;
        }
        else
        {
            Opcode_ = Opcode;
            OpcodeSize_ = 1;
        }

        return true;
    }

    bool VMInstruction::SetOperand(uint8_t* ImmediateBytes, size_t ImmediateSize)
    {
        if (ImmediateSize == 0 ||
            ImmediateSize == 1 || ImmediateSize == 2 ||
            ImmediateSize == 4 || ImmediateSize == 8)
        {
            DASSERT(ImmediateSize <= sizeof(Immediate_));
            if (ImmediateSize)
            {
                std::memcpy(&Immediate_, ImmediateBytes, ImmediateSize);
                OperandCount_ = 1;
            }
            else
            {
                OperandCount_ = 0;
            }

            ImmediateSize_ = static_cast<uint8_t>(ImmediateSize);
            return true;
        }

        return false;
    }

    bool VMInstruction::ToBytes(unsigned char* Buffer, size_t Size, size_t* SizeRequired)
    {
        unsigned char* p = Buffer;

        if (!Valid_)
            return false;

        size_t BytecodeSize = 0;

        BytecodeSize += OpcodeSize_;
        if (OperandCount_)
        {
            for (int i = 0; i < OperandCount_; i++)
                BytecodeSize += OperandSize(i);
        }

        if (SizeRequired)
            *SizeRequired = BytecodeSize;

        if (!Buffer)
            return false;

        if (Size < BytecodeSize)
            return false;

        if (OpcodeSize_ > 1)
        {
            DASSERT(OpcodeSize_ == 2);

            // 
            // Bytecode1[6:0] = Opcode[6:0]
            // IF Opcode[15:7] != 0
            //  Bytecode1[7] = 1
            //  Bytecode2[7:0] = Opcode[14:7]
            // FI
            // 

            *p++ = (Opcode_ & 0x7f) | 0x80;
            *p++ = (Opcode_ >> 7) & 0xff;
        }
        else
        {
            *p++ = Opcode_ & 0x7f;
        }

        for (int i = 0; i < ImmediateSize_; i++)
            *p++ = Immediate_[i];

        DASSERT(p - Buffer == BytecodeSize);

        return true;
    }

    bool VMInstruction::ToMnemonic(char* Buffer, size_t Size, size_t* SizeRequired)
    {
        if (!Valid_)
            return false;

        DASSERT(0 <= Opcode_ && Opcode_ < std::size(InstructionList));

        const auto& Entry = InstructionList[Opcode_];

        char String[64]{};
        strcpy_s(String, Entry.Mnemonic.c_str());

        if (Entry.Operands.size())
        {
            strcat_s(String, " ");

            for (int i = 0; i < Entry.Operands.size(); i++)
            {
                char Value[32];

                if (i > 0)
                    strcat_s(String, ", ");

                switch (Entry.Operands[i])
                {
                case OperandType::T::Imm8:
                {
                    uint8_t Imm = 0;
                    DASSERT(Operand(i, Imm));
                    sprintf_s(Value, "0x%02hhx", Imm);
                    break;
                }
                case OperandType::T::Imm16:
                {
                    uint16_t Imm = 0;
                    DASSERT(Operand(i, Imm));
                    sprintf_s(Value, "0x%04hx", Imm);
                    break;
                }
                case OperandType::T::Imm32:
                {
                    uint32_t Imm = 0;
                    DASSERT(Operand(i, Imm));
                    sprintf_s(Value, "0x%08x", Imm);
                    break;
                }
                case OperandType::T::Imm64:
                {
                    uint64_t Imm = 0;
                    DASSERT(Operand(i, Imm));
                    sprintf_s(Value, "0x%016llx", Imm);
                    break;
                }
                default:
                    DASSERT(false);
                }

                strcat_s(String, Value);
            }
        }

        if (Buffer)
        {
            strcpy_s(Buffer, Size, String);
        }

        if (SizeRequired)
        {
            *SizeRequired = std::strlen(String);
        }

        return true;
    }

    size_t VMInstruction::Decode(uint8_t* Bytecode, size_t Size, VMInstruction* BytecodeOp)
    {
        auto p = Bytecode;
        size_t Remaining = Size;
        constexpr uint8_t OpcodeExtendBit = 0x80;

        if (!Remaining)
            return 0;

        uint16_t Opcode = 0;

        // Opcode (Decoded)    Decode Representation    Bytecode Representation
        // 0000..007f       ->      00..7f           -> -- 00 to -- 7f           (Op[6:0] -> Bytecode[6:0])
        // 0080..7fff       -> 80 | 01..7f, 00..ff   -> 81 00 to ff ff

        // 
        // Bytecode1[6:0] = Opcode[6:0]
        // IF Opcode[15:7] != 0
        //  Bytecode1[7] = 1
        //  Bytecode2[7:0] = Opcode[14:7]
        // FI
        // 

        if (*p & OpcodeExtendBit)
        {
            // 2-byte opcode
            if (Remaining < 2)
                return 0;

            if (p[1] & OpcodeExtendBit)
                return 0; // bad encoding

            Opcode = (p[0] & ~OpcodeExtendBit) | (p[1] << 7ui8);

            p += 2;
            Remaining -= 2;
        }
        else
        {
            // 1-byte opcode
            Opcode = p[0];

            p++;
            Remaining--;
        }

        const auto& Instruction = InstructionList[Opcode];
        DASSERT(Instruction.Id == Opcode);
        DASSERT(Instruction.Operands.size() <= 1);

        VMInstruction DecodedOp = VMInstruction(static_cast<Opcode::T>(Opcode));

        // Check operators
        size_t OperandSize = 0;
        char OperandString[32]{};

        for (auto& it : Instruction.Operands)
        {
            switch (it)
            {
            case OperandType::T::Imm8: OperandSize = 1; break;
            case OperandType::T::Imm16: OperandSize = 2; break;
            case OperandType::T::Imm32: OperandSize = 4; break;
            case OperandType::T::Imm64: OperandSize = 8; break;
            default: DASSERT(false);
            }

            if (Remaining < OperandSize)
                return 0;

            switch (it)
            {
            case OperandType::T::Imm8:
                DecodedOp = VMInstruction::Create(static_cast<Opcode::T>(Opcode), Base::FromBytesLe<uint8_t>(p));
                break;
            case OperandType::T::Imm16:
                DecodedOp = VMInstruction::Create(static_cast<Opcode::T>(Opcode), Base::FromBytesLe<uint16_t>(p));
                break;
            case OperandType::T::Imm32:
                DecodedOp = VMInstruction::Create(static_cast<Opcode::T>(Opcode), Base::FromBytesLe<uint32_t>(p));
                break;
            case OperandType::T::Imm64:
                DecodedOp = VMInstruction::Create(static_cast<Opcode::T>(Opcode), Base::FromBytesLe<uint64_t>(p));
                break;
            default:
                DASSERT(false);
            }

            Remaining -= OperandSize;
        }

        if (BytecodeOp)
            *BytecodeOp = DecodedOp;

        return Size - Remaining;
    }

}

