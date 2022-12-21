

#ifndef INST_TABLE_H_
#define INST_TABLE_H_

#define INSTRUCTION_BASE_PARAMS(_op, _mnemonic)	\
	Opcode::T::_op, #_mnemonic

#define DEFINE_INST(_op, _mnemonic)	\
	{ INSTRUCTION_BASE_PARAMS(_op, _mnemonic), },

#define DEFINE_INST_O1(_op, _mnemonic, _operand)	\
	{ INSTRUCTION_BASE_PARAMS(_op, _mnemonic), { OperandType::T::_operand } },



#include "inst_table.inc" // instruction table



#undef DEFINE_INST_O1
#undef DEFINE_INST
#undef INSTRUCTION_BASE_PARAMS

#undef INST_TABLE_H_ // undef to reuse
#endif

