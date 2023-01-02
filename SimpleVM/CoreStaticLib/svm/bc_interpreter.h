
#pragma once

#include "base.h"
#include "integer.h"
#include "vmmemory.h"

namespace VM_NAMESPACE
{
    class VMBytecodeInterpreter
    {
        using StackType = VMStack;
        using VMPointerType = Base::ToIntegralType<decltype(VMExecutionContext::IP)>::type;

        constexpr const static int StackNativePushSize32 = sizeof(int32_t);
        constexpr const static int StackNativePushSize64 = sizeof(int64_t);

    public:
        VMBytecodeInterpreter(VMMemoryManager& MemoryManager) :
            MemoryManager_(MemoryManager)
        {
        }


        static bool RaiseException(VMExecutionContext& Context, ExceptionState::T State)
        {
            Context.ExceptionState = State;
            Context.NextIP = Context.IP;
            return true;
        }

        static ExceptionState::T IntegerStateToException(uint8_t IntegerState, uint32_t PrefixBits)
        {
            if (IntegerState & Integer<int8_t>::StateFlags::Invalid)
            {
                if (IntegerState & Integer<int8_t>::StateFlags::DivideByZero)
                {
                    return ExceptionState::T::IntegerDivideByZero;
                }

                return ExceptionState::T::InvalidInstruction;
            }

            if ((IntegerState & Integer<int8_t>::StateFlags::Overflow) &&
                (PrefixBits & InstructionPrefixBits::T::CheckOverflow))
            {
                return ExceptionState::T::IntegerOverflow;
            }

            return ExceptionState::T::None;
        }

        int Execute(VMExecutionContext& Context, int Count)
        {
            VMInstruction Op;
            size_t FetchSize = 0;
            int StepCount = 0;

            printf(" =========== VM Execute (Step %10d) =========== \n", Count);

            do
            {
                if (StepCount >= Count)
                    break;

                if (Context.ExceptionState != ExceptionState::T::None)
                    break;

                uintptr_t FetchAddress = MemoryManager_.HostAddress(Context.IP);
                FetchSize = VMInstruction::Decode(
                    reinterpret_cast<uint8_t*>(FetchAddress),
                    VMInstruction::InstructionMaximumSize,
                    &Op);

                if (!FetchSize)
                {
                    // Failed to decode...
                    RaiseException(Context, ExceptionState::T::InvalidInstruction);
                    break;
                }

                DASSERT(Op.Valid());

#if 1 // debug
                char Mnemonic[64];
                DASSERT(Op.ToMnemonic(Mnemonic, std::size(Mnemonic), nullptr));

                unsigned char Bytes[32];
                size_t BytesSize = 0;
                DASSERT(Op.ToBytes(Bytes, std::size(Bytes), &BytesSize));
                DASSERT(BytesSize == FetchSize);

                char BytecodeDump[200]{};
                for (size_t i = 0; i < BytesSize; i++)
                {
                    char Value[10];
                    sprintf_s(Value, "%02hhx ", Bytes[i]);
                    strcat_s(BytecodeDump, Value);
                }

                printf("%-30s%s\n", BytecodeDump, Mnemonic);
#endif

                bool Result = true;

                // Calculate next IP before execution
                Context.NextIP = Base::IntegerAssertCast<VMPointerType>(Context.IP + FetchSize);


                switch (auto Opcode = Op.Opcode())
                {
                case Opcode::T::Add_I4:
                {
                    Result = Inst_Add<int32_t>(Context);
                    break;
                }
                case Opcode::T::Add_I8:
                {
                    Result = Inst_Add<int64_t>(Context);
                    break;
                }
                case Opcode::T::Add_U4:
                {
                    Result = Inst_Add<uint32_t>(Context);
                    break;
                }
                case Opcode::T::Add_U8:
                {
                    Result = Inst_Add<uint64_t>(Context);
                    break;
                }
                case Opcode::T::Add_F4:
                {
                    Result = Inst_Add<float>(Context);
                    break;
                }
                case Opcode::T::Add_F8:
                {
                    Result = Inst_Add<double>(Context);
                    break;
                }

                case Opcode::T::Sub_I4:
                {
                    Result = Inst_Sub<int32_t>(Context);
                    break;
                }
                case Opcode::T::Sub_I8:
                {
                    Result = Inst_Sub<int64_t>(Context);
                    break;
                }
                case Opcode::T::Sub_U4:
                {
                    Result = Inst_Sub<uint32_t>(Context);
                    break;
                }
                case Opcode::T::Sub_U8:
                {
                    Result = Inst_Sub<uint64_t>(Context);
                    break;
                }

                case Opcode::T::Sub_F4:
                {
                    Result = Inst_Sub<float>(Context);
                    break;
                }
                case Opcode::T::Sub_F8:
                {
                    Result = Inst_Sub<double>(Context);
                    break;
                }

                case Opcode::T::Mul_I4:
                {
                    Result = Inst_Mul<int32_t>(Context);
                    break;
                }
                case Opcode::T::Mul_I8:
                {
                    Result = Inst_Mul<int64_t>(Context);
                    break;
                }
                case Opcode::T::Mul_U4:
                {
                    Result = Inst_Mul<uint32_t>(Context);
                    break;
                }
                case Opcode::T::Mul_U8:
                {
                    Result = Inst_Mul<int64_t>(Context);
                    break;
                }
                case Opcode::T::Mul_F4:
                {
                    Result = Inst_Mul<float>(Context);
                    break;
                }
                case Opcode::T::Mul_F8:
                {
                    Result = Inst_Mul<double>(Context);
                    break;
                }


                case Opcode::T::Mulh_I4:
                {
                    Result = Inst_Mulh<int32_t>(Context);
                    break;
                }
                case Opcode::T::Mulh_I8:
                {
                    Result = Inst_Mulh<int64_t>(Context);
                    break;
                }
                case Opcode::T::Mulh_U4:
                {
                    Result = Inst_Mulh<uint32_t>(Context);
                    break;
                }
                case Opcode::T::Mulh_U8:
                {
                    Result = Inst_Mulh<uint64_t>(Context);
                    break;
                }


                case Opcode::T::Div_I4:
                {
                    Result = Inst_Div<int32_t>(Context);
                    break;
                }
                case Opcode::T::Div_I8:
                {
                    Result = Inst_Div<int64_t>(Context);
                    break;
                }
                case Opcode::T::Div_U4:
                {
                    Result = Inst_Div<uint32_t>(Context);
                    break;
                }
                case Opcode::T::Div_U8:
                {
                    Result = Inst_Div<uint64_t>(Context);
                    break;
                }
                case Opcode::T::Div_F4:
                {
                    Result = Inst_Div<float>(Context);
                    break;
                }
                case Opcode::T::Div_F8:
                {
                    Result = Inst_Div<double>(Context);
                    break;
                }

                case Opcode::T::Mod_I4:
                {
                    Result = Inst_Mod<int32_t>(Context);
                    break;
                }
                case Opcode::T::Mod_I8:
                {
                    Result = Inst_Mod<int64_t>(Context);
                    break;
                }
                case Opcode::T::Mod_U4:
                {
                    Result = Inst_Mod<uint32_t>(Context);
                    break;
                }
                case Opcode::T::Mod_U8:
                {
                    Result = Inst_Mod<uint64_t>(Context);
                    break;
                }
                case Opcode::T::Mod_F4:
                {
                    Result = Inst_Mod<float>(Context);
                    break;
                }
                case Opcode::T::Mod_F8:
                {
                    Result = Inst_Mod<double>(Context);
                    break;
                }

                case Opcode::T::Shl_I4:
                {
                    Result = Inst_Shl<int32_t>(Context);
                    break;
                }
                case Opcode::T::Shl_I8:
                {
                    Result = Inst_Shl<int64_t>(Context);
                    break;
                }
                case Opcode::T::Shl_U4:
                {
                    Result = Inst_Shl<uint32_t>(Context);
                    break;
                }
                case Opcode::T::Shl_U8:
                {
                    Result = Inst_Shl<uint64_t>(Context);
                    break;
                }

                case Opcode::T::Shr_I4:
                {
                    Result = Inst_Shr<int32_t>(Context);
                    break;
                }
                case Opcode::T::Shr_I8:
                {
                    Result = Inst_Shr<int64_t>(Context);
                    break;
                }
                case Opcode::T::Shr_U4:
                {
                    Result = Inst_Shr<uint32_t>(Context);
                    break;
                }
                case Opcode::T::Shr_U8:
                {
                    Result = Inst_Shr<uint64_t>(Context);
                    break;
                }

                case Opcode::T::And_X4:
                {
                    Result = Inst_And<uint32_t>(Context);
                    break;
                }
                case Opcode::T::And_X8:
                {
                    Result = Inst_And<uint64_t>(Context);
                    break;
                }

                case Opcode::T::Or_X4:
                {
                    Result = Inst_Or<uint32_t>(Context);
                    break;
                }
                case Opcode::T::Or_X8:
                {
                    Result = Inst_Or<uint64_t>(Context);
                    break;
                }

                case Opcode::T::Xor_X4:
                {
                    Result = Inst_Xor<uint32_t>(Context);
                    break;
                }
                case Opcode::T::Xor_X8:
                {
                    Result = Inst_Xor<uint64_t>(Context);
                    break;
                }

                case Opcode::T::Not_X4:
                {
                    Result = Inst_Not<uint32_t>(Context);
                    break;
                }
                case Opcode::T::Not_X8:
                {
                    Result = Inst_Not<uint64_t>(Context);
                    break;
                }

                case Opcode::T::Neg_I4:
                {
                    Result = Inst_Neg<int32_t>(Context);
                    break;
                }
                case Opcode::T::Neg_I8:
                {
                    Result = Inst_Neg<int64_t>(Context);
                    break;
                }
                case Opcode::T::Neg_F4:
                {
                    Result = Inst_Neg<float>(Context);
                    break;
                }
                case Opcode::T::Neg_F8:
                {
                    Result = Inst_Neg<double>(Context);
                    break;
                }

                case Opcode::T::Abs_I4:
                {
                    Result = Inst_Abs<int32_t>(Context);
                    break;
                }
                case Opcode::T::Abs_I8:
                {
                    Result = Inst_Abs<int64_t>(Context);
                    break;
                }
                case Opcode::T::Abs_F4:
                {
                    Result = Inst_Abs<float>(Context);
                    break;
                }
                case Opcode::T::Abs_F8:
                {
                    Result = Inst_Abs<double>(Context);
                    break;
                }

                case Opcode::T::Cvt2i_F4_I4:
                {
                    Result = Inst_Cvt2i<float, int32_t>(Context);
                    break;
                }
                case Opcode::T::Cvt2i_F4_I8:
                {
                    Result = Inst_Cvt2i<float, int64_t>(Context);
                    break;
                }
                case Opcode::T::Cvt2i_F8_I4:
                {
                    Result = Inst_Cvt2i<double, int32_t>(Context);
                    break;
                }
                case Opcode::T::Cvt2i_F8_I8:
                {
                    Result = Inst_Cvt2i<double, int64_t>(Context);
                    break;
                }

                case Opcode::T::Cvt2f_I4_F4:
                {
                    Result = Inst_Cvt2f<int32_t, float>(Context);
                    break;
                }
                case Opcode::T::Cvt2f_I4_F8:
                {
                    Result = Inst_Cvt2f<int32_t, double>(Context);
                    break;
                }
                case Opcode::T::Cvt2f_I8_F4:
                {
                    Result = Inst_Cvt2f<int64_t, float>(Context);
                    break;
                }
                case Opcode::T::Cvt2f_I8_F8:
                {
                    Result = Inst_Cvt2f<int64_t, double>(Context);
                    break;
                }

                case Opcode::T::Cvtff_F4_F8:
                {
                    Result = Inst_Cvtff<float, double>(Context);
                    break;
                }
                case Opcode::T::Cvtff_F8_F4:
                {
                    Result = Inst_Cvtff<double, float>(Context);
                    break;
                }

                case Opcode::T::Cvt_I4_I1:
                {
                    Result = Inst_Cvt<int32_t, int8_t>(Context);
                    break;
                }
                case Opcode::T::Cvt_I4_I2:
                {
                    Result = Inst_Cvt<int32_t, int16_t>(Context);
                    break;
                }
                case Opcode::T::Cvt_I4_I8:
                {
                    Result = Inst_Cvt<int32_t, int64_t>(Context);
                    break;
                }
                case Opcode::T::Cvt_I4_U1:
                {
                    Result = Inst_Cvt<int32_t, uint8_t>(Context);
                    break;
                }
                case Opcode::T::Cvt_I4_U2:
                {
                    Result = Inst_Cvt<int32_t, uint16_t>(Context);
                    break;
                }
                case Opcode::T::Cvt_I4_U4:
                {
                    Result = Inst_Cvt<int32_t, uint32_t>(Context);
                    break;
                }
                case Opcode::T::Cvt_I4_U8:
                {
                    Result = Inst_Cvt<int32_t, uint64_t>(Context);
                    break;
                }

                case Opcode::T::Cvt_U4_I1:
                {
                    Result = Inst_Cvt<uint32_t, int8_t>(Context);
                    break;
                }
                case Opcode::T::Cvt_U4_I2:
                {
                    Result = Inst_Cvt<uint32_t, int16_t>(Context);
                    break;
                }
                case Opcode::T::Cvt_U4_I4:
                {
                    Result = Inst_Cvt<uint32_t, int32_t>(Context);
                    break;
                }
                case Opcode::T::Cvt_U4_I8:
                {
                    Result = Inst_Cvt<uint32_t, int64_t>(Context);
                    break;
                }
                case Opcode::T::Cvt_U4_U1:
                {
                    Result = Inst_Cvt<uint32_t, uint8_t>(Context);
                    break;
                }
                case Opcode::T::Cvt_U4_U2:
                {
                    Result = Inst_Cvt<uint32_t, uint16_t>(Context);
                    break;
                }
                case Opcode::T::Cvt_U4_U8:
                {
                    Result = Inst_Cvt<uint32_t, uint64_t>(Context);
                    break;
                }

                case Opcode::T::Cvt_I8_I4:
                {
                    Result = Inst_Cvt<int64_t, int32_t>(Context);
                    break;
                }
                case Opcode::T::Cvt_I8_U4:
                {
                    Result = Inst_Cvt<int64_t, uint32_t>(Context);
                    break;
                }
                case Opcode::T::Cvt_I8_U8:
                {
                    Result = Inst_Cvt<int64_t, uint64_t>(Context);
                    break;
                }
                case Opcode::T::Cvt_U8_I4:
                {
                    Result = Inst_Cvt<uint64_t, int32_t>(Context);
                    break;
                }
                case Opcode::T::Cvt_U8_U4:
                {
                    Result = Inst_Cvt<uint64_t, uint32_t>(Context);
                    break;
                }
                case Opcode::T::Cvt_U8_I8:
                {
                    Result = Inst_Cvt<uint64_t, int64_t>(Context);
                    break;
                }

                case Opcode::T::Ldimm_I1:
                {
                    int8_t Operand1{};
                    DASSERT(Op.Operand(0, Operand1));
                    Result = Inst_Ldimm<decltype(Operand1)>(Context, Operand1);
                    break;
                }
                case Opcode::T::Ldimm_I2:
                {
                    int16_t Operand1{};
                    DASSERT(Op.Operand(0, Operand1));
                    Result = Inst_Ldimm<decltype(Operand1)>(Context, Operand1);
                    break;
                }
                case Opcode::T::Ldimm_I4:
                {
                    int32_t Operand1{};
                    DASSERT(Op.Operand(0, Operand1));
                    Result = Inst_Ldimm<decltype(Operand1)>(Context, Operand1);
                    break;
                }
                case Opcode::T::Ldimm_I8:
                {
                    int64_t Operand1{};
                    DASSERT(Op.Operand(0, Operand1));
                    Result = Inst_Ldimm<decltype(Operand1)>(Context, Operand1);
                    break;
                }

                case Opcode::T::Ldarg:
                {
                    uint16_t Operand1{};
                    DASSERT(Op.Operand(0, Operand1));
                    Result = Inst_Ldarg(Context, Operand1, MemoryManager_);
                    break;
                }
                case Opcode::T::Ldvar:
                {
                    uint16_t Operand1{};
                    DASSERT(Op.Operand(0, Operand1));
                    Result = Inst_Ldvar(Context, Operand1, MemoryManager_);
                    break;
                }
                case Opcode::T::Starg:
                {
                    uint16_t Operand1{};
                    DASSERT(Op.Operand(0, Operand1));
                    Result = Inst_Starg(Context, Operand1, MemoryManager_);
                    break;
                }
                case Opcode::T::Stvar:
                {
                    uint16_t Operand1{};
                    DASSERT(Op.Operand(0, Operand1));
                    Result = Inst_Stvar(Context, Operand1, MemoryManager_);
                    break;
                }

                case Opcode::T::Dup:
                {
                    Result = Inst_Dup_Template(Context);
                    break;
                }
                case Opcode::T::Dup2:
                {
                    // duplicate 2 elements
                    Result = Inst_Dup2_Template(Context);
                    break;
                }

                case Opcode::T::Xch:
                {
                    Result = Inst_Xch_Template(Context);
                    break;
                }

                case Opcode::T::Ldvarp:
                {
                    uint16_t Operand1{};
                    DASSERT(Op.Operand(0, Operand1));
                    Result = Inst_Ldvarp(Context, Operand1);
                    break;
                }
                case Opcode::T::Ldargp:
                {
                    uint16_t Operand1{};
                    DASSERT(Op.Operand(0, Operand1));
                    Result = Inst_Ldargp(Context, Operand1);
                    break;
                }

                case Opcode::T::Ldpv_X1:
                {
                    Result = Inst_Ldpv_Template<uint8_t>(Context, MemoryManager_);
                    break;
                }
                case Opcode::T::Ldpv_X2:
                {
                    Result = Inst_Ldpv_Template<uint16_t>(Context, MemoryManager_);
                    break;
                }
                case Opcode::T::Ldpv_X4:
                {
                    Result = Inst_Ldpv_Template<uint32_t>(Context, MemoryManager_);
                    break;
                }
                case Opcode::T::Ldpv_X8:
                {
                    Result = Inst_Ldpv_Template<uint64_t>(Context, MemoryManager_);
                    break;
                }

                case Opcode::T::Stpv_X1:
                {
                    Result = Inst_Stpv_Template<uint8_t>(Context, MemoryManager_);
                    break;
                }
                case Opcode::T::Stpv_X2:
                {
                    Result = Inst_Stpv_Template<uint16_t>(Context, MemoryManager_);
                    break;
                }
                case Opcode::T::Stpv_X4:
                {
                    Result = Inst_Stpv_Template<uint32_t>(Context, MemoryManager_);
                    break;
                }
                case Opcode::T::Stpv_X8:
                {
                    Result = Inst_Stpv_Template<uint64_t>(Context, MemoryManager_);
                    break;
                }

                case Opcode::T::Ppcpy:
                {
                    Result = Inst_Ppcpy_Template(Context, MemoryManager_);
                    break;
                }

                case Opcode::T::Pvfil_X1:
                {
                    Result = Inst_Pvfil_Template<uint8_t>(Context, MemoryManager_);
                    break;
                }
                case Opcode::T::Pvfil_X2:
                {
                    Result = Inst_Pvfil_Template<uint16_t>(Context, MemoryManager_);
                    break;
                }
                case Opcode::T::Pvfil_X4:
                {
                    Result = Inst_Pvfil_Template<uint32_t>(Context, MemoryManager_);
                    break;
                }
                case Opcode::T::Pvfil_X8:
                {
                    Result = Inst_Pvfil_Template<uint64_t>(Context, MemoryManager_);
                    break;
                }

                case Opcode::T::Initarg:
                {
                    Result = Inst_Initarg(Context);
                    break;
                }
                case Opcode::T::Arg:
                {
                    uint32_t Operand1{};
                    DASSERT(Op.Operand(0, Operand1));
                    Result = Inst_Arg(Context, Operand1);
                    break;
                }
                case Opcode::T::Var:
                {
                    uint32_t Operand1{};
                    DASSERT(Op.Operand(0, Operand1));
                    Result = Inst_Var(Context, Operand1);
                    break;
                }

                case Opcode::T::Dcv:
                {
                    Result = Inst_Dcv_Template(Context);
                    break;
                }
                case Opcode::T::Dcvn:
                {
                    Result = Inst_Dcvn_Template(Context);
                    break;
                }

                case Opcode::T::Test_e_I4:
                {
                    Result = Inst_Test_e<int32_t>(Context);
                    break;
                }
                case Opcode::T::Test_e_I8:
                {
                    Result = Inst_Test_e<int64_t>(Context);
                    break;
                }
                case Opcode::T::Test_e_F4:
                {
                    Result = Inst_Test_e<float>(Context);
                    break;
                }
                case Opcode::T::Test_e_F8:
                {
                    Result = Inst_Test_e<double>(Context);
                    break;
                }

                case Opcode::T::Test_ne_I4:
                {
                    Result = Inst_Test_ne<int32_t>(Context);
                    break;
                }
                case Opcode::T::Test_ne_I8:
                {
                    Result = Inst_Test_ne<int64_t>(Context);
                    break;
                }
                case Opcode::T::Test_ne_F4:
                {
                    Result = Inst_Test_ne<float>(Context);
                    break;
                }
                case Opcode::T::Test_ne_F8:
                {
                    Result = Inst_Test_ne<double>(Context);
                    break;
                }

                case Opcode::T::Test_le_I4:
                {
                    Result = Inst_Test_le<int32_t>(Context);
                    break;
                }
                case Opcode::T::Test_le_I8:
                {
                    Result = Inst_Test_le<int64_t>(Context);
                    break;
                }
                case Opcode::T::Test_le_U4:
                {
                    Result = Inst_Test_le<uint32_t>(Context);
                    break;
                }
                case Opcode::T::Test_le_U8:
                {
                    Result = Inst_Test_le<uint64_t>(Context);
                    break;
                }
                case Opcode::T::Test_le_F4:
                {
                    Result = Inst_Test_le<float>(Context);
                    break;
                }
                case Opcode::T::Test_le_F8:
                {
                    Result = Inst_Test_le<double>(Context);
                    break;
                }

                case Opcode::T::Test_ge_I4:
                {
                    Result = Inst_Test_ge<int32_t>(Context);
                    break;
                }
                case Opcode::T::Test_ge_I8:
                {
                    Result = Inst_Test_ge<int64_t>(Context);
                    break;
                }
                case Opcode::T::Test_ge_U4:
                {
                    Result = Inst_Test_ge<uint32_t>(Context);
                    break;
                }
                case Opcode::T::Test_ge_U8:
                {
                    Result = Inst_Test_ge<uint64_t>(Context);
                    break;
                }
                case Opcode::T::Test_ge_F4:
                {
                    Result = Inst_Test_ge<float>(Context);
                    break;
                }
                case Opcode::T::Test_ge_F8:
                {
                    Result = Inst_Test_ge<double>(Context);
                    break;
                }

                case Opcode::T::Test_l_I4:
                {
                    Result = Inst_Test_l<int32_t>(Context);
                    break;
                }
                case Opcode::T::Test_l_I8:
                {
                    Result = Inst_Test_l<int64_t>(Context);
                    break;
                }
                case Opcode::T::Test_l_U4:
                {
                    Result = Inst_Test_l<uint32_t>(Context);
                    break;
                }
                case Opcode::T::Test_l_U8:
                {
                    Result = Inst_Test_l<uint32_t>(Context);
                    break;
                }
                case Opcode::T::Test_l_F4:
                {
                    Result = Inst_Test_l<float>(Context);
                    break;
                }
                case Opcode::T::Test_l_F8:
                {
                    Result = Inst_Test_l<double>(Context);
                    break;
                }

                case Opcode::T::Test_g_I4:
                {
                    Result = Inst_Test_g<int32_t>(Context);
                    break;
                }
                case Opcode::T::Test_g_I8:
                {
                    Result = Inst_Test_g<int64_t>(Context);
                    break;
                }
                case Opcode::T::Test_g_U4:
                {
                    Result = Inst_Test_g<uint32_t>(Context);
                    break;
                }
                case Opcode::T::Test_g_U8:
                {
                    Result = Inst_Test_g<uint64_t>(Context);
                    break;
                }
                case Opcode::T::Test_g_F4:
                {
                    Result = Inst_Test_g<float>(Context);
                    break;
                }
                case Opcode::T::Test_g_F8:
                {
                    Result = Inst_Test_g<double>(Context);
                    break;
                }


                case Opcode::T::Br_I1:
                {
                    int8_t Offset{};
                    DASSERT(Op.Operand(0, Offset));
                    Result = Inst_Br(Offset, Context);
                    break;
                }
                case Opcode::T::Br_I2:
                {
                    int16_t Offset{};
                    DASSERT(Op.Operand(0, Offset));
                    Result = Inst_Br(Offset, Context);
                    break;
                }
                case Opcode::T::Br_I4:
                {
                    int32_t Offset{};
                    DASSERT(Op.Operand(0, Offset));
                    Result = Inst_Br(Offset, Context);
                    break;
                }

                case Opcode::T::Br_z_I1:
                {
                    int8_t Offset{};
                    DASSERT(Op.Operand(0, Offset));
                    Result = Inst_Br_z_Template(Offset, Context);
                    break;
                }
                case Opcode::T::Br_z_I2:
                {
                    int16_t Offset{};
                    DASSERT(Op.Operand(0, Offset));
                    Result = Inst_Br_z_Template(Offset, Context);
                    break;
                }
                case Opcode::T::Br_z_I4:
                {
                    int32_t Offset{};
                    DASSERT(Op.Operand(0, Offset));
                    Result = Inst_Br_z_Template(Offset, Context);
                    break;
                }

                case Opcode::T::Br_nz_I1:
                {
                    int8_t Offset{};
                    DASSERT(Op.Operand(0, Offset));
                    Result = Inst_Br_nz_Template(Offset, Context);
                    break;
                }
                case Opcode::T::Br_nz_I2:
                {
                    int16_t Offset{};
                    DASSERT(Op.Operand(0, Offset));
                    Result = Inst_Br_nz_Template(Offset, Context);
                    break;
                }
                case Opcode::T::Br_nz_I4:
                {
                    int32_t Offset{};
                    DASSERT(Op.Operand(0, Offset));
                    Result = Inst_Br_nz_Template(Offset, Context);
                    break;
                }

                case Opcode::T::Call_I1:
                {
                    int8_t Offset{};
                    DASSERT(Op.Operand(0, Offset));
                    Result = Inst_Call(Offset, Context);
                    break;
                }
                case Opcode::T::Call_I2:
                {
                    int16_t Offset{};
                    DASSERT(Op.Operand(0, Offset));
                    Result = Inst_Call(Offset, Context);
                    break;
                }
                case Opcode::T::Call_I4:
                {
                    int32_t Offset{};
                    DASSERT(Op.Operand(0, Offset));
                    Result = Inst_Call(Offset, Context);
                    break;
                }

                case Opcode::T::Ret:
                {
                    Result = Inst_Ret(Context);
                    break;
                }
                case Opcode::T::Nop:
                {
                    break; // do nothing
                }
                case Opcode::T::Bp:
                {
                    Result = Inst_Bp(Context);
                    break;
                }

                case Opcode::T::Ldvmsr:
                {
                    uint16_t Operand1{};
                    DASSERT(Op.Operand(0, Operand1));
                    Result = Inst_Ldvmsr(Context, Operand1);
                    break;
                }
                case Opcode::T::Stvmsr:
                {
                    uint16_t Operand1{};
                    DASSERT(Op.Operand(0, Operand1));
                    Result = Inst_Stvmsr(Context, Operand1);
                    break;
                }

                case Opcode::T::Vmcall:
                case Opcode::T::Vmxthrow:
                    break;

                default:
                {
                    DASSERT(false);
                }

                }

                DASSERT(Result);

                Context.IP = Context.NextIP;
                StepCount++;
            }
            while (FetchSize);

            printf(" =========== VM Returned (Step %10d) =========== \n\n", StepCount);

            return StepCount;
        }


    private:

        //
        // Instruction templates.
        //

        template <
            typename T,
            std::enable_if_t<std::is_integral<T>::value, bool> = true>
            inline static bool Inst_Add(VMExecutionContext& Context)
        {
            T Op1{}, Op2{};

            if (!Context.Stack.Pop(&Op2) ||
                !Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            Integer<T> Value = Integer<T>(Op1) + Integer<T>(Op2);

            ExceptionState::T  Exception = IntegerStateToException(Value.State(), Context.FetchedPrefix);
            if (Exception != ExceptionState::T::None)
            {
                RaiseException(Context, Exception);
                return false;
            }

            if (!Context.Stack.Push(Value.Value()))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
            inline static bool Inst_Add(VMExecutionContext& Context)
        {
            T Op1{}, Op2{};

            if (!Context.Stack.Pop(&Op2) ||
                !Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            auto Value = Op1 + Op2;

            if (!Context.Stack.Push(Value))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            std::enable_if_t<std::is_integral<T>::value, bool> = true>
            inline static bool Inst_Sub(VMExecutionContext& Context)
        {
            T Op1{}, Op2{};

            if (!Context.Stack.Pop(&Op2) ||
                !Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            Integer<T> Value = Integer<T>(Op1) - Integer<T>(Op2);

            ExceptionState::T  Exception = IntegerStateToException(Value.State(), Context.FetchedPrefix);
            if (Exception != ExceptionState::T::None)
            {
                RaiseException(Context, Exception);
                return false;
            }

            if (!Context.Stack.Push(Value.Value()))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
            inline static bool Inst_Sub(VMExecutionContext& Context)
        {
            T Op1{}, Op2{};

            if (!Context.Stack.Pop(&Op2) ||
                !Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            auto Value = Op1 - Op2;

            if (!Context.Stack.Push(Value))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            std::enable_if_t<std::is_integral<T>::value, bool> = true>
            inline static bool Inst_Mul(VMExecutionContext& Context)
        {
            T Op1{}, Op2{};

            if (!Context.Stack.Pop(&Op2) ||
                !Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            Integer<T> Value = Integer<T>(Op1) * Integer<T>(Op2);

            ExceptionState::T  Exception = IntegerStateToException(Value.State(), Context.FetchedPrefix);
            if (Exception != ExceptionState::T::None)
            {
                RaiseException(Context, Exception);
                return false;
            }

            if (!Context.Stack.Push(Value.Value()))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
            inline static bool Inst_Mul(VMExecutionContext& Context)
        {
            T Op1{}, Op2{};

            if (!Context.Stack.Pop(&Op2) ||
                !Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            auto Value = Op1 * Op2;

            if (!Context.Stack.Push(Value))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value>>
            inline static bool Inst_Mulh(VMExecutionContext& Context)
        {
            T Op1{}, Op2{};

            if (!Context.Stack.Pop(&Op2) ||
                !Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            Integer<T> Result;
            Integer<T>(Op1).Multiply(Op2, &Result);

            ExceptionState::T  Exception = IntegerStateToException(Result.State(), Context.FetchedPrefix);
            if (Exception != ExceptionState::T::None)
            {
                RaiseException(Context, Exception);
                return false;
            }

            if (!Context.Stack.Push(Result.Value()))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            std::enable_if_t<std::is_integral<T>::value, bool> = true>
            inline static bool Inst_Div(VMExecutionContext& Context)
        {
            T Op1{}, Op2{};

            if (!Context.Stack.Pop(&Op2) ||
                !Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            Integer<T> Value = Integer<T>(Op1) / Integer<T>(Op2);

            ExceptionState::T  Exception = IntegerStateToException(Value.State(), Context.FetchedPrefix);
            if (Exception != ExceptionState::T::None)
            {
                RaiseException(Context, Exception);
                return false;
            }

            if (!Context.Stack.Push(Value.Value()))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
            inline static bool Inst_Div(VMExecutionContext& Context)
        {
            T Op1{}, Op2{};

            if (!Context.Stack.Pop(&Op2) ||
                !Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            auto Value = Op1 / Op2;

            if (!Context.Stack.Push(Value))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            std::enable_if_t<std::is_integral<T>::value, bool> = true>
            inline static bool Inst_Mod(VMExecutionContext& Context)
        {
            T Op1{}, Op2{};

            if (!Context.Stack.Pop(&Op2) ||
                !Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            // 
            // We define Mod() for every Op1, Op2 ¡ô Z:
            //   Mod(Op1, Op2) = Op1 - Floor(Op1/Op2)             [Case 1. if Op1 >= 0 and Op2 > 0]
            //   Mod(Op1, Op2) = Undefined                        [Case 2. if Op2 == 0]
            //   Mod(Op1, Op2) = Sgn(Op1*Op2) * Mod(|Op1|, |Op2|) [Case 3. if Op1 < 0 or Op2 < 0]
            // 

            Integer<T> Value = Integer<T>(Op1) % Integer<T>(Op2);

            ExceptionState::T  Exception = IntegerStateToException(Value.State(), Context.FetchedPrefix);
            if (Exception != ExceptionState::T::None)
            {
                RaiseException(Context, Exception);
                return false;
            }

            if (!Context.Stack.Push(Value.Value()))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
            inline static bool Inst_Mod(VMExecutionContext& Context)
        {
            T Op1{}, Op2{};

            if (!Context.Stack.Pop(&Op2) ||
                !Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            int Type1 = std::fpclassify(Op1);
            int Type2 = std::fpclassify(Op2);

            if ((Type1 != FP_NORMAL && Type1 != FP_SUBNORMAL && Type1 != FP_ZERO) ||
                (Type2 != FP_NORMAL && Type2 != FP_SUBNORMAL))
            {
                RaiseException(Context, ExceptionState::T::FloatingPointInvalid);
                return false;
            }

            // 
            // We define Mod() for every Op1, Op2 ¡ô R:
            //   Mod(Op1, Op2) = Op1 - Op1/Op2                    [Case 1. if Op1 >= 0 and Op2 > 0]
            //   Mod(Op1, Op2) = Undefined                        [Case 2. if |Op2| == 0]
            //   Mod(Op1, Op2) = Sgn(Op1*Op2) * Mod(|Op1|, |Op2|) [Case 3. if Op1 < 0 or Op2 < 0]
            // 

            T AbsOp1 = Op1 < 0 ? -Op1 : Op1;
            T AbsOp2 = Op2 < 0 ? -Op2 : Op2;

            if (Type1 == FP_ZERO)
                AbsOp1 = 0;

            DASSERT(Type2 != FP_ZERO);

            T Value = AbsOp1 - AbsOp1 / AbsOp2;

            if (bool ResultNegative = (Op1 < 0) != (Op2 < 0))
            {
                Value = -Value;
            }

            if (!Context.Stack.Push(Value))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value>>
            inline static bool Inst_Shl(VMExecutionContext& Context)
        {
            T Op1{}, Op2{};

            if (!Context.Stack.Pop(&Op2) ||
                !Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            //
            // We define shift left operation shl(a, b) as:
            //   shl(a, b) = a << b (if b is in range [0, sizeof(a)*8))
            //   shl(a, b) = 0      (if b > sizeof(a)*8)
            //   shl(a, b) = #INV   (if b < 0)
            // 
            // where #INV means invalid instruction exception.
            //

            Integer<T> Value = Integer<T>(Op1) << Integer<T>(Op2);

            ExceptionState::T  Exception = IntegerStateToException(Value.State(), Context.FetchedPrefix);
            if (Exception != ExceptionState::T::None)
            {
                RaiseException(Context, Exception);
                return false;
            }

            if (!Context.Stack.Push(Value.Value()))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value>>
            inline static bool Inst_Shr(VMExecutionContext& Context)
        {
            T Op1{}, Op2{};

            if (!Context.Stack.Pop(&Op2) ||
                !Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            //
            // We define shift right operation shr(a, b) as:
            //   shr(a, b) = a >> b (if b is in range [0, sizeof(a)*8))
            //   shr(a, b) = 0      (if b > sizeof(a)*8)
            //   shr(a, b) = #INV   (if b < 0)
            // 
            // where #INV means invalid instruction exception.
            //

            Integer<T> Value = Integer<T>(Op1) >> Integer<T>(Op2);

            ExceptionState::T  Exception = IntegerStateToException(Value.State(), Context.FetchedPrefix);
            if (Exception != ExceptionState::T::None)
            {
                RaiseException(Context, Exception);
                return false;
            }

            if (!Context.Stack.Push(Value.Value()))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value>>
            inline static bool Inst_And(VMExecutionContext& Context)
        {
            T Op1{}, Op2{};

            if (!Context.Stack.Pop(&Op2) ||
                !Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            Integer<T> Value = Integer<T>(Op1) & Integer<T>(Op2);

            ExceptionState::T  Exception = IntegerStateToException(Value.State(), Context.FetchedPrefix);
            if (Exception != ExceptionState::T::None)
            {
                RaiseException(Context, Exception);
                return false;
            }

            if (!Context.Stack.Push(Value.Value()))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value>>
            inline static bool Inst_Or(VMExecutionContext& Context)
        {
            T Op1{}, Op2{};

            if (!Context.Stack.Pop(&Op2) ||
                !Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            Integer<T> Value = Integer<T>(Op1) | Integer<T>(Op2);

            ExceptionState::T  Exception = IntegerStateToException(Value.State(), Context.FetchedPrefix);
            if (Exception != ExceptionState::T::None)
            {
                RaiseException(Context, Exception);
                return false;
            }

            if (!Context.Stack.Push(Value.Value()))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value>>
            inline static bool Inst_Xor(VMExecutionContext& Context)
        {
            T Op1{}, Op2{};

            if (!Context.Stack.Pop(&Op2) ||
                !Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            Integer<T> Value = Integer<T>(Op1) ^ Integer<T>(Op2);

            ExceptionState::T  Exception = IntegerStateToException(Value.State(), Context.FetchedPrefix);
            if (Exception != ExceptionState::T::None)
            {
                RaiseException(Context, Exception);
                return false;
            }

            if (!Context.Stack.Push(Value.Value()))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value>>
            inline static bool Inst_Not(VMExecutionContext& Context)
        {
            T Op1{};

            if (!Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            Integer<T> Value = ~Integer<T>(Op1);

            ExceptionState::T  Exception = IntegerStateToException(Value.State(), Context.FetchedPrefix);
            if (Exception != ExceptionState::T::None)
            {
                RaiseException(Context, Exception);
                return false;
            }

            if (!Context.Stack.Push(Value.Value()))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            std::enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value, bool> = true>
            inline static bool Inst_Neg(VMExecutionContext& Context)
        {
            T Op1{};

            if (!Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            Integer<T> Value = -Integer<T>(Op1);

            ExceptionState::T  Exception = IntegerStateToException(Value.State(), Context.FetchedPrefix);
            if (Exception != ExceptionState::T::None)
            {
                RaiseException(Context, Exception);
                return false;
            }

            if (!Context.Stack.Push(Value.Value()))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
            inline static bool Inst_Neg(VMExecutionContext& Context)
        {
            T Op1{};

            if (!Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            T Value = -Op1;

            if (!Context.Stack.Push(Value))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }


        template <
            typename T,
            std::enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value, bool> = true>
            inline static bool Inst_Abs(VMExecutionContext& Context)
        {
            T Op1{};

            if (!Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            Integer<T> Value = Op1;
            if (Value.Value() < 0)
                Value = -Value;

            if (!Context.Stack.Push(Value.Value()))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
            inline static bool Inst_Abs(VMExecutionContext& Context)
        {
            T Op1{};

            if (!Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            auto Value = std::abs(Op1);
            static_assert(std::is_same<decltype(Value), T>::value,
                "return type of std::abs() is different");

            if (!Context.Stack.Push(Value))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename TFloat,	// floating point type
            typename TInteger,	// integer type
            typename = std::enable_if_t<
            std::is_floating_point<TFloat>::value&& std::is_integral<TInteger>::value>>
            inline static bool Inst_Cvt2i(VMExecutionContext& Context)
        {
            TFloat Op1{};

            if (!Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            auto Value = static_cast<TInteger>(Op1);

            if (!Context.Stack.Push(Value))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename TInteger,	// integer type
            typename TFloat,	// floating point type
            typename = std::enable_if_t<
            std::is_integral<TInteger>::value&& std::is_floating_point<TFloat>::value>>
            inline static bool Inst_Cvt2f(VMExecutionContext& Context)
        {
            TInteger Op1{};

            if (!Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            auto Value = static_cast<TFloat>(Op1);

            if (!Context.Stack.Push(Value))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T1,	// floating point type
            typename T2,	// floating point type
            typename = std::enable_if_t<
            std::is_floating_point<T1>::value&& std::is_floating_point<T2>::value>>
            inline static bool Inst_Cvtff(VMExecutionContext& Context)
        {
            T1 Op1{};

            if (!Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            auto Value = static_cast<T2>(Op1);

            if (!Context.Stack.Push(Value))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T1,	// integer type
            typename T2,	// integer type
            typename = std::enable_if_t<
            std::is_integral<T1>::value&& std::is_integral<T2>::value>>
            inline static bool Inst_Cvt(VMExecutionContext& Context)
        {
            T1 Op1{};

            if (!Context.Stack.Pop(&Op1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            // src		dest		conversion
            // signed	signed		(dest)v
            // signed	unsigned	(dest)(src.unsigned)v
            // unsigned	signed		(dest)(src.signed)v
            // unsigned	unsigned	(dest)v

            using IntermediateType = std::conditional_t<
                std::is_signed<T1>::value == std::is_signed<T2>::value,
                T2, // both T1 and T2 are signed or unsigned
                std::conditional_t<  // mixed type (signed/unsigned)
                std::is_signed<T1>::value,
                std::make_unsigned<T1>::type, // make unsigned if signed
                std::make_signed<T1>::type> // make signed if unsigned
            >;

            auto Value = static_cast<T2>(static_cast<IntermediateType>(Op1));

            if (!Context.Stack.Push(Value))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value>>
            inline static bool Inst_Ldimm(VMExecutionContext& Context, T Value)
        {
            if (!Context.Stack.Push(Value))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value>>
            inline static bool Inst_Ldarg(VMExecutionContext& Context, T Index, VMMemoryManager& Memory)
        {
            ShadowFrame Frame{};
            if (!Context.ShadowStack.PeekFrom(&Frame, 0))
            {
                // Shadow stack overflow
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            using TableEntryType = ArgumentTableEntry;
            VMStack& XStack = Context.ArgumentStack;

            const auto EntrySize = sizeof(TableEntryType);
            uint32_t CurrentPointer = Base::IntegerAssertCast<uint32_t>(XStack.TopOffset());
            uint32_t FramePointerBase = Frame.ATP;
            uint32_t OffsetFromFrameBase = FramePointerBase - CurrentPointer;

            DASSERT(!(EntrySize % XStack.Alignment()));
            uint32_t EntryCount = OffsetFromFrameBase / EntrySize;

            if (!(FramePointerBase >= CurrentPointer) ||
                (OffsetFromFrameBase % EntrySize))
            {
                // Strange ATP
                RaiseException(Context, ExceptionState::T::InvalidAccess);
                return false;
            }

            if (!(Index < EntryCount))
            {
                // Invalid argument index
                RaiseException(Context, ExceptionState::T::InvalidInstruction);
                return false;
            }

            TableEntryType Entry{};
            uint32_t OffsetFromCurrentPointer = (Index - (EntryCount - 1)) * EntrySize;
            if (!XStack.PeekFrom(&Entry, OffsetFromCurrentPointer))
            {
                RaiseException(Context, ExceptionState::T::InvalidInstruction);
                return false;
            }
            DASSERT(false); // NOTE: verification needed

            // Hi |   ...    |   <- Frame.ATP points here
            //    +----------+
            //    | ArgEntry1| \
            //    +----------+  |
            //    | ArgEntry2|  |
            //    +----------+  ArgumentFrame
            //    |   ...    |  |
            //    +----------+  |
            //    | ArgEntryN| /  <- ATP points here
            //    +----------+
            // Lo |   ...    |

            auto Source = Memory.HostAddress(Entry.Address, Entry.Size);
            if (!Source)
            {
                // access is out of bound
                RaiseException(Context, ExceptionState::T::InvalidAccess);
                return false;
            }

            if (!Context.Stack.Push(reinterpret_cast<unsigned char*>(Source), Entry.Size))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value>>
            inline static bool Inst_Ldvar(VMExecutionContext& Context, T Index, VMMemoryManager& Memory)
        {
            ShadowFrame Frame{};
            if (!Context.ShadowStack.PeekFrom(&Frame, 0))
            {
                // Shadow stack overflow
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            using TableEntryType = LocalVariableTableEntry;
            VMStack& XStack = Context.LocalVariableStack;

            const auto EntrySize = sizeof(TableEntryType);
            uint32_t CurrentPointer = Base::IntegerAssertCast<uint32_t>(XStack.TopOffset());
            uint32_t FramePointerBase = Frame.LVTP;
            uint32_t OffsetFromFrameBase = FramePointerBase - CurrentPointer;

            DASSERT(!(EntrySize % XStack.Alignment()));
            uint32_t EntryCount = OffsetFromFrameBase / EntrySize;

            if (!(FramePointerBase >= CurrentPointer) ||
                (OffsetFromFrameBase % EntrySize))
            {
                // Strange LVTP
                RaiseException(Context, ExceptionState::T::InvalidAccess);
                return false;
            }

            if (!(Index < EntryCount))
            {
                // Invalid local variable index
                RaiseException(Context, ExceptionState::T::InvalidInstruction);
                return false;
            }

            TableEntryType Entry{};
            uint32_t OffsetFromCurrentPointer = (Index - (EntryCount - 1)) * EntrySize;
            if (!XStack.PeekFrom(&Entry, OffsetFromCurrentPointer))
            {
                RaiseException(Context, ExceptionState::T::InvalidInstruction);
                return false;
            }
            DASSERT(false); // NOTE: verification needed

            // Hi |   ...    |   <- Frame.LVTP points here
            //    +----------+
            //    | ArgEntry1| \
            //    +----------+  |
            //    | ArgEntry2|  |
            //    +----------+  LocalVariableFrame
            //    |   ...    |  |
            //    +----------+  |
            //    | ArgEntryN| /  <- LVTP points here
            //    +----------+
            // Lo |   ...    |

            auto Source = Memory.HostAddress(Entry.Address, Entry.Size);
            if (!Source)
            {
                // access is out of bound
                RaiseException(Context, ExceptionState::T::InvalidAccess);
                return false;
            }

            if (!Context.Stack.Push(reinterpret_cast<unsigned char*>(Source), Entry.Size))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value>>
            inline static bool Inst_Starg(VMExecutionContext& Context, T Index, VMMemoryManager& Memory)
        {
            ShadowFrame Frame{};
            if (!Context.ShadowStack.PeekFrom(&Frame, 0))
            {
                // Shadow stack overflow
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            using TableEntryType = ArgumentTableEntry;
            VMStack& XStack = Context.ArgumentStack;

            const auto EntrySize = sizeof(TableEntryType);
            uint32_t CurrentPointer = Base::IntegerAssertCast<uint32_t>(XStack.TopOffset());
            uint32_t FramePointerBase = Frame.ATP;
            uint32_t OffsetFromFrameBase = FramePointerBase - CurrentPointer;

            DASSERT(!(EntrySize % XStack.Alignment()));
            uint32_t EntryCount = OffsetFromFrameBase / EntrySize;

            if (!(FramePointerBase >= CurrentPointer) ||
                (OffsetFromFrameBase % EntrySize))
            {
                // Strange ATP
                RaiseException(Context, ExceptionState::T::InvalidAccess);
                return false;
            }

            if (!(Index < EntryCount))
            {
                // Invalid argument index
                RaiseException(Context, ExceptionState::T::InvalidInstruction);
                return false;
            }

            TableEntryType Entry{};
            uint32_t OffsetFromCurrentPointer = (Index - (EntryCount - 1)) * EntrySize;
            if (!XStack.PeekFrom(&Entry, OffsetFromCurrentPointer))
            {
                RaiseException(Context, ExceptionState::T::InvalidInstruction);
                return false;
            }
            DASSERT(false); // NOTE: verification needed

            auto Dest = Memory.HostAddress(Entry.Address, Entry.Size);
            if (!Dest)
            {
                // access is out of bound
                RaiseException(Context, ExceptionState::T::InvalidAccess);
                return false;
            }

            unsigned char Temp[128];
            unsigned char* TempPtr = Temp;
            std::unique_ptr<unsigned char[]> TempLargeBuffer;
            if (Entry.Size > sizeof(Temp))
            {
                TempLargeBuffer = std::make_unique<unsigned char[]>(Entry.Size);
                TempPtr = TempLargeBuffer.get();
            }

            if (!TempPtr)
            {
                // not enough memory!
                DASSERT(false);
                return false;
            }

            if (!Context.Stack.Pop(TempPtr, Entry.Size))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            std::memcpy(reinterpret_cast<unsigned char*>(Dest), TempPtr, Entry.Size);

            return true;
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value>>
            inline static bool Inst_Stvar(VMExecutionContext& Context, T Index, VMMemoryManager& Memory)
        {
            ShadowFrame Frame{};
            if (!Context.ShadowStack.PeekFrom(&Frame, 0))
            {
                // Shadow stack overflow
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            using TableEntryType = LocalVariableTableEntry;
            VMStack& XStack = Context.LocalVariableStack;

            const auto EntrySize = sizeof(TableEntryType);
            uint32_t CurrentPointer = Base::IntegerAssertCast<uint32_t>(XStack.TopOffset());
            uint32_t FramePointerBase = Frame.LVTP;
            uint32_t OffsetFromFrameBase = FramePointerBase - CurrentPointer;

            DASSERT(!(EntrySize % XStack.Alignment()));
            uint32_t EntryCount = OffsetFromFrameBase / EntrySize;

            if (!(FramePointerBase >= CurrentPointer) ||
                (OffsetFromFrameBase % EntrySize))
            {
                // Strange LVTP
                RaiseException(Context, ExceptionState::T::InvalidAccess);
                return false;
            }

            if (!(Index < EntryCount))
            {
                // Invalid local variable index
                RaiseException(Context, ExceptionState::T::InvalidInstruction);
                return false;
            }

            TableEntryType Entry{};
            uint32_t OffsetFromCurrentPointer = (Index - (EntryCount - 1)) * EntrySize;
            if (!XStack.PeekFrom(&Entry, OffsetFromCurrentPointer))
            {
                RaiseException(Context, ExceptionState::T::InvalidInstruction);
                return false;
            }
            DASSERT(false); // NOTE: verification needed

            auto Dest = Memory.HostAddress(Entry.Address, Entry.Size);
            if (!Dest)
            {
                // access is out of bound
                RaiseException(Context, ExceptionState::T::InvalidAccess);
                return false;
            }

            unsigned char Temp[128];
            unsigned char* TempPtr = Temp;
            std::unique_ptr<unsigned char[]> TempLargeBuffer;
            if (Entry.Size > sizeof(Temp))
            {
                TempLargeBuffer = std::make_unique<unsigned char[]>(Entry.Size);
                TempPtr = TempLargeBuffer.get();
            }

            if (!TempPtr)
            {
                // not enough memory!
                DASSERT(false);
                return false;
            }

            if (!Context.Stack.Pop(TempPtr, Entry.Size))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            std::memcpy(reinterpret_cast<unsigned char*>(Dest), TempPtr, Entry.Size);

            return true;
        }

        inline static bool Inst_Dup_Template(VMExecutionContext& Context)
        {
            if (IsStackOper64Bit(Context))
            {
                return Inst_Dup<uint64_t>(Context);
            }
            else
            {
                return Inst_Dup<uint32_t>(Context);
            }
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>>
            inline static bool Inst_Dup(VMExecutionContext& Context)
        {
            T Value{};
            if (!Context.Stack.PeekFrom(&Value, 0))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            if (!Context.Stack.Push(Value) ||
                !Context.Stack.Push(Value))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        inline static bool Inst_Dup2_Template(VMExecutionContext& Context)
        {
            if (IsStackOper64Bit(Context))
            {
                return Inst_Dup2<uint64_t>(Context);
            }
            else
            {
                return Inst_Dup2<uint32_t>(Context);
            }
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>>
            inline static bool Inst_Dup2(VMExecutionContext& Context)
        {
            T Value1, Value2{};
            if (!Context.Stack.Pop(&Value2) ||
                !Context.Stack.Pop(&Value1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            if (!Context.Stack.Push(Value1) ||
                !Context.Stack.Push(Value2) ||
                !Context.Stack.Push(Value1) ||
                !Context.Stack.Push(Value2))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        inline static bool Inst_Xch_Template(VMExecutionContext& Context)
        {
            if (IsStackOper64Bit(Context))
            {
                return Inst_Xch<uint64_t>(Context);
            }
            else
            {
                return Inst_Xch<uint32_t>(Context);
            }
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>>
            inline static bool Inst_Xch(VMExecutionContext& Context)
        {
            T Value1{}, Value2{};

            if (!Context.Stack.Pop(&Value1) ||
                !Context.Stack.Pop(&Value2))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            if (!Context.Stack.Push(Value2) ||
                !Context.Stack.Push(Value1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value>>
            inline static bool Inst_Ldargp(VMExecutionContext& Context, T Index)
        {
            ShadowFrame Frame{};
            if (!Context.ShadowStack.PeekFrom(&Frame, 0))
            {
                // Shadow stack overflow
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            using TableEntryType = ArgumentTableEntry;
            VMStack& XStack = Context.ArgumentStack;

            const auto EntrySize = sizeof(TableEntryType);
            uint32_t CurrentPointer = Base::IntegerAssertCast<uint32_t>(XStack.TopOffset());
            uint32_t FramePointerBase = Frame.ATP;
            uint32_t OffsetFromFrameBase = FramePointerBase - CurrentPointer;

            DASSERT(!(EntrySize % XStack.Alignment()));
            uint32_t EntryCount = OffsetFromFrameBase / EntrySize;

            if (!(FramePointerBase >= CurrentPointer) ||
                (OffsetFromFrameBase % EntrySize))
            {
                // Strange ATP
                RaiseException(Context, ExceptionState::T::InvalidAccess);
                return false;
            }

            if (!(Index < EntryCount))
            {
                // Invalid argument index
                RaiseException(Context, ExceptionState::T::InvalidInstruction);
                return false;
            }

            TableEntryType Entry{};
            uint32_t OffsetFromCurrentPointer = (Index - (EntryCount - 1)) * EntrySize;
            if (!XStack.PeekFrom(&Entry, OffsetFromCurrentPointer))
            {
                RaiseException(Context, ExceptionState::T::InvalidInstruction);
                return false;
            }
            DASSERT(false); // NOTE: verification needed

            if (!Context.Stack.Push(Entry.Address) ||
                !Context.Stack.Push(Entry.Size))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value>>
            inline static bool Inst_Ldvarp(VMExecutionContext& Context, T Index)
        {
            ShadowFrame Frame{};
            if (!Context.ShadowStack.PeekFrom(&Frame, 0))
            {
                // Shadow stack overflow
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            using TableEntryType = LocalVariableTableEntry;
            VMStack& XStack = Context.LocalVariableStack;

            const auto EntrySize = sizeof(TableEntryType);
            uint32_t CurrentPointer = Base::IntegerAssertCast<uint32_t>(XStack.TopOffset());
            uint32_t FramePointerBase = Frame.LVTP;
            uint32_t OffsetFromFrameBase = FramePointerBase - CurrentPointer;

            DASSERT(!(EntrySize % XStack.Alignment()));
            uint32_t EntryCount = OffsetFromFrameBase / EntrySize;

            if (!(FramePointerBase >= CurrentPointer) ||
                (OffsetFromFrameBase % EntrySize))
            {
                // Strange LVTP
                RaiseException(Context, ExceptionState::T::InvalidAccess);
                return false;
            }

            if (!(Index < EntryCount))
            {
                // Invalid local variable index
                RaiseException(Context, ExceptionState::T::InvalidInstruction);
                return false;
            }

            TableEntryType Entry{};
            uint32_t OffsetFromCurrentPointer = (Index - (EntryCount - 1)) * EntrySize;
            if (!XStack.PeekFrom(&Entry, OffsetFromCurrentPointer))
            {
                RaiseException(Context, ExceptionState::T::InvalidInstruction);
                return false;
            }
            DASSERT(false); // NOTE: verification needed

            if (!Context.Stack.Push(Entry.Address) ||
                !Context.Stack.Push(Entry.Size))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename TValue,
            typename = std::enable_if_t<std::is_integral<TValue>::value>>
            inline static bool Inst_Ldpv_Template(VMExecutionContext& Context, VMMemoryManager& Memory)
        {
            if (IsStackOper64Bit(Context))
            {
                return Inst_Ldpv<TValue, uint64_t>(Context, Memory);
            }
            else
            {
                return Inst_Ldpv<TValue, uint32_t>(Context, Memory);
            }
        }

        template <
            typename TValue,
            typename TPointer,
            typename = std::enable_if_t<
            std::is_integral<TValue>::value&& std::is_integral<TPointer>::value>>
            inline static bool Inst_Ldpv(VMExecutionContext& Context, VMMemoryManager& Memory)
        {
            TPointer Reference{};
            TValue Value{};

            if (!Context.Stack.Pop(&Reference))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            if (Memory.Read(Reference, sizeof(TValue), reinterpret_cast<uint8_t*>(&Value)) != sizeof(TValue))
            {
                RaiseException(Context, ExceptionState::T::InvalidAccess);
                return false;
            }

            if (!Context.Stack.Push(Value))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename TValue,
            typename = std::enable_if_t<std::is_integral<TValue>::value>>
            inline static bool Inst_Stpv_Template(VMExecutionContext& Context, VMMemoryManager& Memory)
        {
            if (IsStackOper64Bit(Context))
            {
                return Inst_Stpv<TValue, uint64_t>(Context, Memory);
            }
            else
            {
                return Inst_Stpv<TValue, uint32_t>(Context, Memory);
            }
        }

        template <
            typename TValue,
            typename TPointer,
            typename = std::enable_if_t<
            std::is_integral<TValue>::value&& std::is_integral<TPointer>::value>>
            inline static bool Inst_Stpv(VMExecutionContext& Context, VMMemoryManager& Memory)
        {
            TPointer Reference{};
            TValue Value{};

            if (!Context.Stack.Pop(&Value) ||
                !Context.Stack.Pop(&Reference))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            if (Memory.Write(Reference, sizeof(TValue), reinterpret_cast<uint8_t*>(&Value)) != sizeof(TValue))
            {
                RaiseException(Context, ExceptionState::T::InvalidAccess);
                return false;
            }

            return true;
        }

        inline static bool Inst_Ppcpy_Template(VMExecutionContext& Context, VMMemoryManager& Memory)
        {
            if (IsAddress64Bit(Context))
            {
                return Inst_Ppcpy<int64_t>(Context, Memory);
            }
            else
            {
                return Inst_Ppcpy<int32_t>(Context, Memory);
            }
        }

        template <
            typename TPointer,
            typename = std::enable_if_t<std::is_integral<TPointer>::value>>
            inline static bool Inst_Ppcpy(VMExecutionContext& Context, VMMemoryManager& Memory)
        {
            TPointer Dest{}, Source{}, Size{};

            if (!Context.Stack.Pop(&Size) ||
                !Context.Stack.Pop(&Source) ||
                !Context.Stack.Pop(&Dest))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            auto SourceAddress = Memory.HostAddress(Source, Size);
            auto DestAddress = Memory.HostAddress(Dest, Size);

            if (!SourceAddress || !DestAddress)
            {
                RaiseException(Context, ExceptionState::T::InvalidAccess);
                return false;
            }

            std::memcpy(
                reinterpret_cast<void*>(DestAddress),
                reinterpret_cast<void*>(SourceAddress),
                Size);

            return true;
        }

        template <
            typename TValue,
            typename = std::enable_if_t<std::is_integral<TValue>::value>>
            inline static bool Inst_Pvfil_Template(VMExecutionContext& Context, VMMemoryManager& Memory)
        {
            if (IsAddress64Bit(Context))
            {
                return Inst_Pvfil<int64_t, TValue>(Context, Memory);
            }
            else
            {
                return Inst_Pvfil<int32_t, TValue>(Context, Memory);
            }
        }

        template <
            typename TPointer,
            typename TValue,
            typename = std::enable_if_t<
            std::is_integral<TPointer>::value&& std::is_integral<TValue>::value>>
            inline static bool Inst_Pvfil(VMExecutionContext& Context, VMMemoryManager& Memory)
        {
            TPointer Dest{}, Count{};
            TValue Value{};

            if (!Context.Stack.Pop(&Count) ||
                !Context.Stack.Pop(&Value) ||
                !Context.Stack.Pop(&Dest))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            size_t Size = Count * sizeof(TValue);
            auto DestAddress = Memory.HostAddress(Dest, Size);

            if (!DestAddress)
            {
                RaiseException(Context, ExceptionState::T::InvalidAccess);
                return false;
            }

            switch (sizeof(TValue))
            {
            case 1:
            {
                std::memset(reinterpret_cast<void*>(DestAddress), static_cast<int>(Value), Size);
                break;
            }
            case 2:
            {
                // NOTE: rep stosw/d/q accepts count, not size
                //__stosw(reinterpret_cast<uint16_t*>(DestAddress), static_cast<uint16_t>(Value), Count);
                for (decltype(Count) i = 0; i < Count; i++)
                {
                    Base::ToBytes(Value, reinterpret_cast<unsigned char*>(DestAddress) + i * sizeof(TValue));
                }
                break;
            }
            case 4:
            {
                // NOTE: rep stosw/d/q accepts count, not size
                //__stosd(reinterpret_cast<DWORD*>(DestAddress), static_cast<uint32_t>(Value), Count);
                for (decltype(Count) i = 0; i < Count; i++)
                {
                    Base::ToBytes(Value, reinterpret_cast<unsigned char*>(DestAddress) + i * sizeof(TValue));
                }
                break;
            }
            case 8:
            {
                // NOTE: rep stosw/d/q accepts count, not size
                //__stosq(reinterpret_cast<uint64_t*>(DestAddress), Value, Count);
                for (decltype(Count) i = 0; i < Count; i++)
                {
                    Base::ToBytes(Value, reinterpret_cast<unsigned char*>(DestAddress) + i * sizeof(TValue));
                }
                break;
            }
            }

            return true;
        }

        inline static bool Inst_Initarg(VMExecutionContext& Context)
        {
            ShadowFrame Frame{};
            if (!Context.ShadowStack.PeekFrom(&Frame, 0))
            {
                // Shadow stack overflow
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            if (!Context.ArgumentStack.SetTopOffset(Frame.ATP))
            {
                // Invalid access (ATP is out of bound)
                RaiseException(Context, ExceptionState::T::InvalidAccess);
                return false;
            }

            // Clear XTS.ArgumentTableReady
            Context.XTableState &= ~XTableStateBits::T::ArgumentTableReady;
            return true;
        }

        inline static bool Inst_Arg(VMExecutionContext& Context, uint32_t Size)
        {
            if (Size == 0 ||
                Size > Constants::MaximumSizeSingleArgument)
            {
                // Argument size is zero or too big
                RaiseException(Context, ExceptionState::T::InvalidInstruction);
                return false;
            }

            ShadowFrame Frame{};
            if (!Context.ShadowStack.PeekFrom(&Frame, 0))
            {
                // Shadow stack overflow
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            uint32_t ATP = Base::IntegerAssertCast<uint32_t>(Context.ArgumentStack.TopOffset());
            uint32_t OffsetFromFrameATP = Frame.ATP - ATP;
            const auto ArgumentEntrySize = sizeof(ArgumentTableEntry);
            DASSERT(!(ArgumentEntrySize % Context.ArgumentStack.Alignment()));
            uint32_t EntryCount = OffsetFromFrameATP / ArgumentEntrySize;

            if (!(Frame.ATP >= ATP) ||
                (OffsetFromFrameATP % ArgumentEntrySize))
            {
                // Strange ATP
                RaiseException(Context, ExceptionState::T::InvalidAccess);
                return false;
            }

            if (EntryCount >= Constants::MaximumFunctionArgumentCount)
            {
                // Maximum argument count exceeded
                RaiseException(Context, ExceptionState::T::InvalidInstruction);
                return false;
            }

            auto SP = Context.Stack.TopOffset();
            if (!(Frame.ReturnSP >= SP))
            {
                // Strange SP
                RaiseException(Context, ExceptionState::T::InvalidAccess);
                return false;
            }

            // Reserve stack
            if (!Context.Stack.Push(nullptr, Size))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            // Add argument entry.
            ArgumentTableEntry Entry{};
            Entry.Size = Size;
            Entry.Address = Base::IntegerAssertCast<uint32_t>(Context.Stack.TopOffset());
            if (!Context.ArgumentStack.Push(Entry))
            {
                RaiseException(Context, ExceptionState::T::InvalidAccess);
                return false;
            }

            // Set XTS.ArgumentTableReady
            Context.XTableState |= XTableStateBits::T::ArgumentTableReady;

            return true;
        }

        inline static bool Inst_Var(VMExecutionContext& Context, uint32_t Size)
        {
            if (Size == 0 ||
                Size > Constants::MaximumSizeSingleLocalVariable)
            {
                // Local variable size is zero or too big
                RaiseException(Context, ExceptionState::T::InvalidInstruction);
                return false;
            }

            ShadowFrame Frame{};
            if (!Context.ShadowStack.PeekFrom(&Frame, 0))
            {
                // Shadow stack overflow
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            uint32_t LVTP = Base::IntegerAssertCast<uint32_t>(Context.LocalVariableStack.TopOffset());
            uint32_t OffsetFromFrameLVTP = Frame.LVTP - LVTP;
            const auto LocalVarEntrySize = sizeof(LocalVariableTableEntry);
            DASSERT(!(LocalVarEntrySize % Context.LocalVariableStack.Alignment()));
            uint32_t EntryCount = OffsetFromFrameLVTP / LocalVarEntrySize;

            if (!(Frame.LVTP >= LVTP) ||
                (OffsetFromFrameLVTP % LocalVarEntrySize))
            {
                // Strange ATP
                RaiseException(Context, ExceptionState::T::InvalidAccess);
                return false;
            }

            if (EntryCount >= Constants::MaximumFunctionLocalVariableCount)
            {
                // Maximum local variable count exceeded
                RaiseException(Context, ExceptionState::T::InvalidInstruction);
                return false;
            }

            auto SP = Context.Stack.TopOffset();
            if (!(Frame.ReturnSP >= SP))
            {
                // Strange SP
                RaiseException(Context, ExceptionState::T::InvalidAccess);
                return false;
            }

            // Reserve stack
            if (!Context.Stack.Push(nullptr, Size))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            // Add local variable entry.
            LocalVariableTableEntry Entry{};
            Entry.Size = Size;
            Entry.Address = Base::IntegerAssertCast<uint32_t>(Context.Stack.TopOffset());
            if (!Context.LocalVariableStack.Push(Entry))
            {
                RaiseException(Context, ExceptionState::T::InvalidAccess);
                return false;
            }

            // Set XTS.LocalVariableTableReady
            Context.XTableState |= XTableStateBits::T::LocalVariableTableReady;

            return true;
        }

        inline static bool Inst_Dcv_Template(VMExecutionContext& Context)
        {
            if (IsStackOper64Bit(Context))
            {
                return Inst_Dcv<int64_t>(Context);
            }
            else
            {
                return Inst_Dcv<int32_t>(Context);
            }
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value>>
            inline static bool Inst_Dcv(VMExecutionContext& Context)
        {
            T Temp{};
            if (!Context.Stack.Pop(&Temp))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        inline static bool Inst_Dcvn_Template(VMExecutionContext& Context)
        {
            if (IsStackOper64Bit(Context))
            {
                return Inst_Dcvn<int64_t>(Context);
            }
            else
            {
                return Inst_Dcvn<int32_t>(Context);
            }
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value>>
            inline static bool Inst_Dcvn(VMExecutionContext& Context)
        {
            T Count{};
            if (!Context.Stack.Pop(&Count))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            T Temp{};
            for (T i = 0; i < Count; i++)
            {
                if (!Context.Stack.Pop(&Temp))
                {
                    RaiseException(Context, ExceptionState::T::StackOverflow);
                    return false;
                }
            }

            return true;
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>>
            inline static bool Inst_Test_e(VMExecutionContext& Context)
        {
            T Value1{}, Value2{};

            if (!Context.Stack.Pop(&Value2) ||
                !Context.Stack.Pop(&Value1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            uint8_t Result = (Value1 == Value2);
            if (!Context.Stack.Push(Result))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>>
            inline static bool Inst_Test_ne(VMExecutionContext& Context)
        {
            T Value1{}, Value2{};

            if (!Context.Stack.Pop(&Value2) ||
                !Context.Stack.Pop(&Value1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            uint8_t Result = (Value1 != Value2);
            if (!Context.Stack.Push(Result))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>>
            inline static bool Inst_Test_le(VMExecutionContext& Context)
        {
            T Value1{}, Value2{};

            if (!Context.Stack.Pop(&Value2) ||
                !Context.Stack.Pop(&Value1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            uint8_t Result = (Value1 <= Value2);
            if (!Context.Stack.Push(Result))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>>
            inline static bool Inst_Test_ge(VMExecutionContext& Context)
        {
            T Value1{}, Value2{};

            if (!Context.Stack.Pop(&Value2) ||
                !Context.Stack.Pop(&Value1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            uint8_t Result = (Value1 >= Value2);
            if (!Context.Stack.Push(Result))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>>
            inline static bool Inst_Test_l(VMExecutionContext& Context)
        {
            T Value1{}, Value2{};

            if (!Context.Stack.Pop(&Value2) ||
                !Context.Stack.Pop(&Value1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            uint8_t Result = (Value1 < Value2);
            if (!Context.Stack.Push(Result))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>>
            inline static bool Inst_Test_g(VMExecutionContext& Context)
        {
            T Value1{}, Value2{};

            if (!Context.Stack.Pop(&Value2) ||
                !Context.Stack.Pop(&Value1))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            uint8_t Result = (Value1 > Value2);
            if (!Context.Stack.Push(Result))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        template <
            typename TOffset,
            typename = std::enable_if_t<std::is_integral<TOffset>::value>>
            inline static bool Inst_Br(TOffset Offset, VMExecutionContext& Context)
        {
            auto RelativeOffset = Base::SignExtend<VMPointerType>(Offset);

            Context.NextIP += RelativeOffset;

            return true;
        }

        template <
            typename TOffset,
            typename = std::enable_if_t<std::is_integral<TOffset>::value>>
            inline static bool Inst_Br_z_Template(TOffset Offset, VMExecutionContext& Context)
        {
            if (IsStackOper64Bit(Context))
            {
                return Inst_Br_z<uint64_t>(Offset, Context);
            }
            else
            {
                return Inst_Br_z<uint32_t>(Offset, Context);
            }
        }

        template <
            typename TCondition,
            typename TOffset,
            typename = std::enable_if_t<
            std::is_integral<TOffset>::value&& std::is_integral<TCondition>::value>>
            inline static bool Inst_Br_z(TOffset Offset, VMExecutionContext& Context)
        {
            auto RelativeOffset = Base::SignExtend<VMPointerType>(Offset);

            TCondition Condition{};
            if (!Context.Stack.Pop(&Condition))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            if (!Condition)
            {
                Context.NextIP += RelativeOffset;
            }

            return true;
        }

        template <
            typename TOffset,
            typename = std::enable_if_t<std::is_integral<TOffset>::value>>
            inline static bool Inst_Br_nz_Template(TOffset Offset, VMExecutionContext& Context)
        {
            if (IsStackOper64Bit(Context))
            {
                return Inst_Br_nz<uint64_t>(Offset, Context);
            }
            else
            {
                return Inst_Br_nz<uint32_t>(Offset, Context);
            }
        }

        template <
            typename TCondition,
            typename TOffset,
            typename = std::enable_if_t<
            std::is_integral<TOffset>::value&& std::is_integral<TCondition>::value>>
            inline static bool Inst_Br_nz(TOffset Offset, VMExecutionContext& Context)
        {
            auto RelativeOffset = Base::SignExtend<VMPointerType>(Offset);

            TCondition Condition{};
            if (!Context.Stack.Pop(&Condition))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            if (!!Condition)
            {
                Context.NextIP += RelativeOffset;
            }

            return true;
        }

        template <
            typename TOffset,
            typename = std::enable_if_t<std::is_integral<TOffset>::value>>
            inline static bool Inst_Call(TOffset Offset, VMExecutionContext& Context)
        {
            auto RelativeOffset = Base::SignExtend<VMPointerType>(Offset);

            auto ReturnIP = Context.NextIP;
            if (!Context.Stack.Push(ReturnIP))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            // build shadow frame
            ShadowFrame Frame;
            Frame.ReturnIP = ReturnIP;
            Frame.ReturnSP = Base::IntegerAssertCast
                <decltype(Frame.ReturnSP)>(Context.Stack.TopOffset());
            Frame.LVTP = Base::IntegerAssertCast
                <decltype(Frame.LVTP)>(Context.LocalVariableStack.TopOffset());
            Frame.ATP = 0;
            Frame.XTableState = 0;

            if (!Context.ShadowStack.Push(Frame))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            Context.NextIP += RelativeOffset;

            return true;
        }

        inline static bool Inst_Ret(VMExecutionContext& Context)
        {
            VMPointerType ReturnIP{};
            if (!Context.Stack.Pop(&ReturnIP))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            // pop shadow frame
            ShadowFrame Frame;
            if (!Context.ShadowStack.Pop(&Frame))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            if (Frame.ReturnIP != ReturnIP)
            {
                RaiseException(Context, ExceptionState::T::InvalidAccess);
                return false;
            }

            Context.NextIP = ReturnIP;

            return true;
        }

        inline static bool Inst_Bp(VMExecutionContext& Context)
        {
            RaiseException(Context, ExceptionState::T::Breakpoint);
            return true;
        }


        inline static bool Inst_Ldvmsr(VMExecutionContext& Context, uint16_t Index)
        {
            if (!(0 <= Index && Index < std::size(Context.VMSR)))
            {
                RaiseException(Context, ExceptionState::T::InvalidInstruction);
                return false;
            }

            if (!Context.Stack.Push(Context.VMSR[Index]))
            {
                RaiseException(Context, ExceptionState::T::StackOverflow);
                return false;
            }

            return true;
        }

        inline static bool Inst_Stvmsr(VMExecutionContext& Context, uint16_t Index)
        {
            if (!(0 <= Index && Index < std::size(Context.VMSR)))
            {
                RaiseException(Context, ExceptionState::T::InvalidInstruction);
                return false;
            }

            // NOTE: Currently all VMSR is read-only
            RaiseException(Context, ExceptionState::T::InvalidInstruction);
            return false;

            //if (!Context.Stack.Pop(&Context.VMSR[Index]))
            //{
            //	RaiseException(Context, ExceptionState::T::StackOverflow);
            //	return false;
            //}

            //return true;
        }

        static bool IsAddress64Bit(const VMExecutionContext& Context) noexcept
        {
            return !!(Context.Mode & ModeBits::T::VMPointer64Bit);
        }

        static bool IsStackOper64Bit(const VMExecutionContext& Context) noexcept
        {
            return !!(Context.Mode & ModeBits::T::VMStackOper64Bit);
        }

        static int VMPointerSize(const VMExecutionContext& Context) noexcept
        {
            if (Context.Mode & ModeBits::T::VMPointer64Bit)
                return sizeof(int64_t);

            return sizeof(int32_t);
        }

        static int VMStackOperSize(const VMExecutionContext& Context) noexcept
        {
            if (Context.Mode & ModeBits::T::VMStackOper64Bit)
                return sizeof(int64_t);

            return sizeof(int32_t);
        }


        VMMemoryManager& MemoryManager_;
    };

}

