
#pragma once

#include "base.h"

namespace VM_NAMESPACE
{
	struct OperandType
	{
		enum T : uint32_t
		{
			None,   // None

			Imm8,   // 8-bit Immediate
			Imm16,  // 16-bit Immediate
			Imm32,  // 32-bit Immediate
			Imm64,  // 64-bit Immediate
		};
	};

	class Operand
	{
	public:
		constexpr Operand() :
			Type(OperandType::T::None), Value() {}
		constexpr Operand(OperandType::T Type, uint64_t Value) :
			Type(Type), Value(Value) {}

		OperandType::T Type;
		uint64_t Value;
	};

	class InstructionInfo
	{
	public:
		InstructionInfo(uint32_t Id, const char* Mnemonic, std::vector<OperandType::T> Operands = {}) :
			Id(Id), Mnemonic(Mnemonic), Operands(Operands)
		{
		}

		InstructionInfo(uint32_t Id, const char* Mnemonic, std::initializer_list<OperandType::T> Operands) :
			InstructionInfo(Id, Mnemonic, std::vector<OperandType::T>(Operands))
		{
		}

		uint32_t Id;
		std::string Mnemonic;
		std::vector<OperandType::T> Operands;
	};


	struct Opcode
	{
		enum T : uint32_t
		{
			Nop,
			Bp,

			Inv,

			Add_I4,
			Add_I8,
			Add_U4,
			Add_U8,
			Add_F4,
			Add_F8,

			Sub_I4,
			Sub_I8,
			Sub_U4,
			Sub_U8,
			Sub_F4,
			Sub_F8,

			Mul_I4,
			Mul_I8,
			Mul_U4,
			Mul_U8,
			Mul_F4,
			Mul_F8,

			Mulh_I4,
			Mulh_I8,
			Mulh_U4,
			Mulh_U8,

			Div_I4,
			Div_I8,
			Div_U4,
			Div_U8,
			Div_F4,
			Div_F8,
			Mod_I4,
			Mod_I8,
			Mod_U4,
			Mod_U8,
			Mod_F4,
			Mod_F8,

			Shl_I4,
			Shl_I8,
			Shl_U4,
			Shl_U8,
			Shr_I4,
			Shr_I8,
			Shr_U4,
			Shr_U8,

			And_X4,
			And_X8,
			Or_X4,
			Or_X8,
			Xor_X4,
			Xor_X8,
			Not_X4,
			Not_X8,
			Neg_I4,
			Neg_I8,
			Neg_F4,
			Neg_F8,
			Abs_I4,
			Abs_I8,
			Abs_F4,
			Abs_F8,

			Cvt2i_F4_I4,
			Cvt2i_F4_I8,
			Cvt2i_F8_I4,
			Cvt2i_F8_I8,
			Cvt2f_I4_F4,
			Cvt2f_I4_F8,
			Cvt2f_I8_F4,
			Cvt2f_I8_F8,
			Cvtff_F4_F8,
			Cvtff_F8_F4,

			Cvt_I1_I4,
			Cvt_I2_I4,
			Cvt_I4_I1,
			Cvt_I4_I2,
			Cvt_I4_I8,
			Cvt_I8_I4,

			Cvt_U1_U4,
			Cvt_U2_U4,
			Cvt_U4_U1,
			Cvt_U4_U2,
			Cvt_U4_U8,
			Cvt_U8_U4,

			Cvt_I1_U1,
			Cvt_I2_U2,
			Cvt_I4_U4,
			Cvt_I8_U8,

			Cvt_U1_I1,
			Cvt_U2_I2,
			Cvt_U4_I4,
			Cvt_U8_I8,

			Ldimm_I1,
			Ldimm_I2,
			Ldimm_I4,
			Ldimm_I8,

			Ldarg,
			Ldvar,
			Starg,
			Stvar,

			Dup,
			Dup2,
			Xch,

			Ldvarp,
			Ldargp,
			Ldpv_X1,
			Ldpv_X2,
			Ldpv_X4,
			Ldpv_X8,
			Stpv_X1,
			Stpv_X2,
			Stpv_X4,
			Stpv_X8,
			Ppcpy,
			Pvfil_X1,
			Pvfil_X2,
			Pvfil_X4,
			Pvfil_X8,

			Initarg,
			Arg,
			Var,

			Dcv,
			Dcvn,

			Test_e_I4,
			Test_e_I8,
			Test_e_F4,
			Test_e_F8,
			Test_ne_I4,
			Test_ne_I8,
			Test_ne_F4,
			Test_ne_F8,
			Test_le_I4,
			Test_le_I8,
			Test_le_U4,
			Test_le_U8,
			Test_le_F4,
			Test_le_F8,
			Test_ge_I4,
			Test_ge_I8,
			Test_ge_U4,
			Test_ge_U8,
			Test_ge_F4,
			Test_ge_F8,
			Test_l_I4,
			Test_l_I8,
			Test_l_U4,
			Test_l_U8,
			Test_l_F4,
			Test_l_F8,
			Test_g_I4,
			Test_g_I8,
			Test_g_U4,
			Test_g_U8,
			Test_g_F4,
			Test_g_F8,

			Br_I1,
			Br_I2,
			Br_I4,
			Br_z_I1,
			Br_z_I2,
			Br_z_I4,
			Br_nz_I1,
			Br_nz_I2,
			Br_nz_I4,

			Call_I1,
			Call_I2,
			Call_I4,
			Ret,

			Ldvmsr,
			Stvmsr,
			Vmcall,
			Vmxthrow,
		};
	};

	class VMInstruction
	{
	public:
		VMInstruction();
		VMInstruction(Opcode::T Opcode, unsigned char* ImmediateBytes = nullptr, size_t ImmediateSize = 0);

		static auto Create(Opcode::T Opcode)
		{
			return VMInstruction(Opcode);
		}

		template <
			typename T,
			typename = std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>>
		static auto Create(Opcode::T Opcode, T Immediate)
		{
			constexpr const auto Size = sizeof(T);
			unsigned char Buffer[Size]{};
			Base::ToBytesLe(Immediate, Buffer);
			return VMInstruction(Opcode, Buffer, Size);
		}

		Opcode::T Opcode() const;
		size_t OpcodeSize() const;
		int OperandCount() const;

		template <
			typename T,
			typename = std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>>
		bool Operand(int Index, T& Result)
		{
			T Value{};
			constexpr const uint8_t ExpectedSize = sizeof(T);
			auto ActualSize = Operand(Index, reinterpret_cast<uint8_t*>(&Value), ExpectedSize);

			if (ExpectedSize != ActualSize)
				return false;

			Result = Value;
			return true;
		}

		uint8_t Operand(int Index, uint8_t* Buffer, uint8_t Size);
		uint8_t OperandSize(int Index) const;
		bool Valid() const;
		bool ToBytes(unsigned char* Buffer, size_t Size, size_t* SizeRequired);
		bool ToMnemonic(char* Buffer, size_t Size, size_t* SizeRequired);

		static size_t Decode(uint8_t* Bytecode, size_t Size, VMInstruction* BytecodeOp);

	private:
		bool SetOpcode(Opcode::T Opcode);
		bool SetOpcode(uint16_t Opcode);

		template <
			typename T,
			typename = std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>>
		bool SetOperand(T Immediate)
		{
			return SetOperand(reinterpret_cast<uint8_t*>(&Immediate), sizeof(T));
		}

		bool SetOperand(uint8_t* ImmediateBytes, size_t ImmediateSize);

		uint16_t Opcode_;
		uint8_t Immediate_[8];
		uint8_t OpcodeSize_;
		uint8_t ImmediateSize_;
		uint8_t OperandCount_;
		bool Valid_;

	public:
		static const InstructionInfo InstructionList[];
		static const size_t InstructionMaximumSize;
	};

}

