

/*
 * add.<type> - add two values
 *
 * expression:
 *     add.<i4|i8|u4|u8|f4|f8>
 * operation:
 *     v2 = PopT(type)
 *     v1 = PopT(type)
 *     PushT(type, v1 + v2)
 * stack changes:
 *     ..., v1, v2 -> ..., v1 + v2
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 */
DEFINE_INST		(Add_I4,		add.i4				)
DEFINE_INST		(Add_I8,		add.i8				)
DEFINE_INST		(Add_U4,		add.u4				)
DEFINE_INST		(Add_U8,		add.u8				)
DEFINE_INST		(Add_F4,		add.f4				)
DEFINE_INST		(Add_F8,		add.f8				)

/*
 * sub.<type> - subtract two values
 *
 * expression:
 *     sub.<i4|i8|u4|u8|f4|f8>
 * operation:
 *     v2 = PopT(type)
 *     v1 = PopT(type)
 *     PushT(type, v1 - v2)
 * stack changes:
 *     ..., v1, v2 -> ..., v1 - v2
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 */
DEFINE_INST		(Sub_I4,		sub.i4				)
DEFINE_INST		(Sub_I8,		sub.i8				)
DEFINE_INST		(Sub_U4,		sub.u4				)
DEFINE_INST		(Sub_U8,		sub.u8				)
DEFINE_INST		(Sub_F4,		sub.f4				)
DEFINE_INST		(Sub_F8,		sub.f8				)

/*
 * mul.<type> - multiply two floating point values
 * mul.<type> - multiply two integer values and get lowword
 *
 * expression:
 *     mul.<i4|i8|u4|u8|f4|f8>
 * operation:
 *     v2 = PopT(type)
 *     v1 = PopT(type)
 *     PushT(type, v1 * v2)
 * stack changes:
 *     ..., v1, v2 -> ..., v1 * v2
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 */
DEFINE_INST		(Mul_I4,		mul.i4				)
DEFINE_INST		(Mul_I8,		mul.i8				)
DEFINE_INST		(Mul_U4,		mul.u4				)
DEFINE_INST		(Mul_U8,		mul.u8				)
DEFINE_INST		(Mul_F4,		mul.f4				)
DEFINE_INST		(Mul_F8,		mul.f8				)

/*
 * mulh.<type> - multiply two values and get highword
 *
 * expression:
 *     mulh.<i4|i8|u4|u8>
 * operation:
 *     v2 = PopT(type)
 *     v1 = PopT(type)
 *     result2x = IntegerMultiply2X(v1, v2)
 *     result_high = HighPart(result2x)
 *     PushT(type, result_high)
 * stack changes:
 *     ..., v1, v2 -> ..., result_high
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 */
DEFINE_INST		(Mulh_I4,		mulh.i4				)
DEFINE_INST		(Mulh_I8,		mulh.i8				)
DEFINE_INST		(Mulh_U4,		mulh.u4				)
DEFINE_INST		(Mulh_U8,		mulh.u8				)

/*
 * div.<type> - divide two values
 *
 * expression:
 *     div.<i4|i8|u4|u8|f4|f8>
 * operation:
 *     v2 = PopT(type)
 *     v1 = PopT(type)
 *     PushT(type, v1 / v2)
 * stack changes:
 *     ..., v1, v2 -> ..., v1 / v2
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 *     #DIV (attempt to divide by zero for integer type)
 */
DEFINE_INST		(Div_I4,		div.i4				)
DEFINE_INST		(Div_I8,		div.i8				)
DEFINE_INST		(Div_U4,		div.u4				)
DEFINE_INST		(Div_U8,		div.u8				)
DEFINE_INST		(Div_F4,		div.f4				)
DEFINE_INST		(Div_F8,		div.f8				)

/*
 * mod.<type> - get remainder
 *
 * expression:
 *     mod.<i4|i8|u4|u8|f4|f8>
 * operation:
 *     v2 = PopT(type)
 *     v1 = PopT(type)
 *     PushT(type, v1 mod v2)
 * stack changes:
 *     ..., v1, v2 -> ..., v1 mod v2
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 *     #DIV (attempt to divide by zero for integer type)
 */
DEFINE_INST		(Mod_I4,		mod.i4				)
DEFINE_INST		(Mod_I8,		mod.i8				)
DEFINE_INST		(Mod_U4,		mod.u4				)
DEFINE_INST		(Mod_U8,		mod.u8				)
DEFINE_INST		(Mod_F4,		mod.f4				)
DEFINE_INST		(Mod_F8,		mod.f8				)

/*
 * shl.<type> - shift left
 *
 * expression:
 *     shl.<i4|i8|u4|u8>
 * operation:
 *     v2 = PopT(type)
 *     v1 = PopT(type)
 *     PushT(type, v1 << v2)
 * stack changes:
 *     ..., v1, v2 -> ..., v1 << v2
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 */
DEFINE_INST		(Shl_I4,		shl.i4				)
DEFINE_INST		(Shl_I8,		shl.i8				)
DEFINE_INST		(Shl_U4,		shl.u4				)
DEFINE_INST		(Shl_U8,		shl.u8				)

/*
 * shr.<type> - shift right
 *
 * expression:
 *     shr.<i4|i8|u4|u8>
 * operation:
 *     v2 = PopT(type)
 *     v1 = PopT(type)
 *     PushT(type, v1 >> v2)
 * stack changes:
 *     ..., v1, v2 -> ..., v1 >> v2
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 */
DEFINE_INST		(Shr_I4,		shr.i4				)
DEFINE_INST		(Shr_I8,		shr.i8				)
DEFINE_INST		(Shr_U4,		shr.u4				)
DEFINE_INST		(Shr_U8,		shr.u8				)

/*
 * and.<type> - bitwise and
 *
 * expression:
 *     and.<x4|x8>
 * operation:
 *     v2 = PopT(type)
 *     v1 = PopT(type)
 *     PushT(type, v1 & v2)
 * stack changes:
 *     ..., v1, v2 -> ..., v1 & v2
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 */
DEFINE_INST		(And_X4,		and.x4				)
DEFINE_INST		(And_X8,		and.x8				)

/*
 * or.<type> - bitwise or
 *
 * expression:
 *     or.<x4|x8>
 * operation:
 *     v2 = PopT(type)
 *     v1 = PopT(type)
 *     PushT(type, v1 | v2)
 * stack changes:
 *     ..., v1, v2 -> ..., v1 | v2
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 */
DEFINE_INST		(Or_X4,			or.x4				)
DEFINE_INST		(Or_X8,			or.x8				)

/*
 * xor.<type> - bitwise xor
 *
 * expression:
 *     xor.<x4|x8>
 * operation:
 *     v2 = PopT(type)
 *     v1 = PopT(type)
 *     PushT(type, v1 ^ v2)
 * stack changes:
 *     ..., v1, v2 -> ..., v1 ^ v2
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 */
DEFINE_INST		(Xor_X4,		xor.x4				)
DEFINE_INST		(Xor_X8,		xor.x8				)

/*
 * not.<type> - bitwise not
 *
 * expression:
 *     not.<x4|x8>
 * operation:
 *     v1 = PopT(type)
 *     PushT(type, ~v1)
 * stack changes:
 *     ..., v1 -> ..., ~v1
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 */
DEFINE_INST		(Not_X4,		not.x4				)
DEFINE_INST		(Not_X8,		not.x8				)

/*
 * neg.<type> - negate value
 *
 * expression:
 *     neg.<i4|i8|f4|f8>
 * operation:
 *     v1 = PopT(type)
 *     PushT(type, -v1)
 * stack changes:
 *     ..., v1 -> ..., -v1
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 */
DEFINE_INST		(Neg_I4,		neg.i4				)
DEFINE_INST		(Neg_I8,		neg.i8				)
DEFINE_INST		(Neg_F4,		neg.f4				)
DEFINE_INST		(Neg_F8,		neg.f8				)

/*
 * abs.<type> - get absolute value
 *
 * expression:
 *     abs.<i4|i8|f4|f8>
 * operation:
 *     v1 = PopT(type)
 *     PushT(type, Abs(v1))
 * stack changes:
 *     ..., v1 -> ..., Abs(v1)
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 */
DEFINE_INST		(Abs_I4,		abs.i4				)
DEFINE_INST		(Abs_I8,		abs.i8				)
DEFINE_INST		(Abs_F4,		abs.f4				)
DEFINE_INST		(Abs_F8,		abs.f8				)

/*
 * cvt2i.<source_type>.<dest_type> - convert floating point to integer
 *
 * expression:
 *     cvt2i.<f4|f8>.<i4|i8>
 * operation:
 *     v1 = PopT(source_type)
 *     PushT(dest_type, FloatToInteger(v1))
 * stack changes:
 *     ..., v1 -> ..., FloatToInteger(v1)
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 */
DEFINE_INST		(Cvt2i_F4_I4,	cvt2i.f4.i4			)
DEFINE_INST		(Cvt2i_F4_I8,	cvt2i.f4.i8			)
DEFINE_INST		(Cvt2i_F8_I4,	cvt2i.f8.i4			)
DEFINE_INST		(Cvt2i_F8_I8,	cvt2i.f8.i8			)

/*
 * cvt2f.<source_type>.<dest_type> - convert integer to floating point
 *
 * expression:
 *     cvt2f.<i4|i8>.<f4|f8>
 * operation:
 *     v1 = PopT(source_type)
 *     PushT(dest_type, IntegerToFloat(v1))
 * stack changes:
 *     ..., v1 -> ..., IntegerToFloat(v1)
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 */
DEFINE_INST		(Cvt2f_I4_F4,	cvt2f.i4.f4			)
DEFINE_INST		(Cvt2f_I4_F8,	cvt2f.i4.f8			)
DEFINE_INST		(Cvt2f_I8_F4,	cvt2f.i8.f4			)
DEFINE_INST		(Cvt2f_I8_F8,	cvt2f.i8.f8			)

/*
 * cvtff.<source_type>.<dest_type> - convert floating point
 *
 * expression:
 *     cvtff.f4.f8 (convert single precision to double precision)
 *     cvtff.f8.f4 (convert double precision to single precision)
 * operation:
 *     v1 = PopT(source_type)
 *     PushT(dest_type, FloatToFloatT(source_type, v1, dest_type))
 * stack changes:
 *     ..., v1 -> ..., FloatToFloatT(source_type, v1, dest_type)
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 */
DEFINE_INST		(Cvtff_F4_F8,	cvtff.f4.f8			)
DEFINE_INST		(Cvtff_F8_F4,	cvtff.f8.f4			)

/*
 * cvt.<source_type>.<dest_type> - convert integer to integer
 *
 * expression:
 *     cvt.i4.<i1|i2|i8|u1|u2|u4|u8> (convert 32-bit signed integer to integer)
 *     cvt.u4.<i1|i2|i4|i8|u1|u2|u8> (convert 32-bit unsigned integer to integer)
 *     cvt.i8.<i4|u4|u8> (convert 64-bit signed integer to integer)
 *     cvt.u8.<i4|u4|i8> (convert 64-bit unsigned integer to integer)
 * operation:
 *     v1 = PopT(source_type)
 *     PushT(dest_type, IntegerToIntegerT(source_type, v1, dest_type))
 * stack changes:
 *     ..., v1 -> ..., IntegerToIntegerT(source_type, v1, dest_type)
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 */
DEFINE_INST		(Cvt_I4_I1,		cvt.i4.i1			)
DEFINE_INST		(Cvt_I4_I2,		cvt.i4.i2			)
DEFINE_INST		(Cvt_I4_I8,		cvt.i4.i8			)
DEFINE_INST		(Cvt_I4_U1,		cvt.i4.u1			)
DEFINE_INST		(Cvt_I4_U2,		cvt.i4.u2			)
DEFINE_INST		(Cvt_I4_U4,		cvt.i4.u4			)
DEFINE_INST		(Cvt_I4_U8,		cvt.i4.u8			)
DEFINE_INST		(Cvt_U4_I1,		cvt.u4.i1			)
DEFINE_INST		(Cvt_U4_I2,		cvt.u4.i2			)
DEFINE_INST		(Cvt_U4_I4,		cvt.u4.i4			)
DEFINE_INST		(Cvt_U4_I8,		cvt.u4.i8			)
DEFINE_INST		(Cvt_U4_U1,		cvt.u4.u1			)
DEFINE_INST		(Cvt_U4_U2,		cvt.u4.u2			)
DEFINE_INST		(Cvt_U4_U8,		cvt.u4.u8			)
DEFINE_INST		(Cvt_I8_I4,		cvt.i8.i4			)
DEFINE_INST		(Cvt_I8_U4,		cvt.i8.u4			)
DEFINE_INST		(Cvt_I8_U8,		cvt.i8.u8			)
DEFINE_INST		(Cvt_U8_I4,		cvt.u8.i4			)
DEFINE_INST		(Cvt_U8_U4,		cvt.u8.u4			)
DEFINE_INST		(Cvt_U8_I8,		cvt.u8.i8			)

/*
 * ldimm.<type> <value> - load immediate value
 *
 * expression:
 *     ldimm.<i1|i2|i4|i8> <value>
 * operation:
 *     PushT(type, value)
 * stack changes:
 *     ... -> ..., value
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 */
DEFINE_INST_O1	(Ldimm_I1,		ldimm.i1,			Imm8	)
DEFINE_INST_O1	(Ldimm_I2,		ldimm.i2,			Imm16	)
DEFINE_INST_O1	(Ldimm_I4,		ldimm.i4,			Imm32	)
DEFINE_INST_O1	(Ldimm_I8,		ldimm.i8,			Imm64	)

/*
 * ldarg <argument_index> - load argument
 *
 * expression:
 *     ldarg <argument_index>
 * operation:
 *     arg = GetArgumentEntry(argument_index)
 *     value = GetArgumentValue(arg.valueref)
 *     PushBytes(arg.size, value)
 * stack changes:
 *     ... -> ..., value
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 *     #INV (invalid argument_index is specified)
 */
DEFINE_INST_O1	(Ldarg,			ldarg,				Imm16	)

/*
 * ldvar <local_variable_index> - load local variable
 *
 * expression:
 *     ldvar <local_variable_index>
 * operation:
 *     local_var = GetLocalVariableEntry(local_variable_index)
 *     value = GetLocalVariableValue(local_var.valueref)
 *     PushBytes(local_var.size, value)
 * stack changes:
 *     ... -> ..., value
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 *     #INV (invalid local_variable_index is specified)
 */
DEFINE_INST_O1	(Ldvar,			ldvar,				Imm16	)

/*
 * starg <argument_index> - store argument
 *
 * expression:
 *     starg <argument_index>
 * operation:
 *     arg = GetArgumentEntry(argument_index)
 *     value = PopBytes(arg.size)
 *     SetArgumentValue(arg.valueref, value)
 * stack changes:
 *     ..., value -> ...
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 *     #INV (invalid argument_index is specified)
 */
DEFINE_INST_O1	(Starg,			starg,				Imm16	)

/*
 * stvar <local_variable_index> - store local variable
 *
 * expression:
 *     stvar <local_variable_index>
 * operation:
 *     local_var = GetLocalVariableEntry(local_variable_index)
 *     value = PopBytes(local_var.size)
 *     SetLocalVariableValue(local_var.valueref, value)
 * stack changes:
 *     ..., value -> ...
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 *     #INV (invalid local_variable_index is specified)
 */
DEFINE_INST_O1	(Stvar,			stvar,				Imm16	)

/*
 * dup - duplicate value
 *
 * expression:
 *     dup
 * operation:
 *     value = PeekN(0) // peek most-recently-pushed 1 element
 *     PushN(value) // push 1 element
 * stack changes:
 *     ..., value -> ..., value, value
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 */
DEFINE_INST		(Dup,			dup					)

/*
 * dup2 - duplicate 2 values
 *
 * expression:
 *     dup2
 * operation:
 *     value1 = PeekN(1)
 *     value2 = PeekN(0)
 *     PushN(value1) // push 1 element
 *     PushN(value2) // push 1 element
 * stack changes:
 *     ..., value1, value2 -> ..., value1, value2, value1, value2
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 */
DEFINE_INST		(Dup2,			dup2				)

/*
 * xch - exchange 2 values
 *
 * expression:
 *     xch
 * operation:
 *     value2 = PopN()
 *     value1 = PopN()
 *     PushN(value2) // push 1 element
 *     PushN(value1) // push 1 element
 * stack changes:
 *     ..., value1, value2 -> ..., value2, value1
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 */
DEFINE_INST		(Xch,			xch					)

/*
 * ldvarp <local_variable_index> - load local variable pointer with size
 *
 * expression:
 *     ldvarp <local_variable_index>
 * operation:
 *     local_var = GetLocalVariableEntry(local_variable_index)
 *     PushN(local_var.valueref)
 *     PushN(local_var.size)
 * stack changes:
 *     ... -> ..., local_var.valueref, local_var.size
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 *     #INV (invalid local_variable_index is specified)
 */
DEFINE_INST_O1	(Ldvarp,		ldvarp,				Imm16	)

/*
 * ldargp <argument_index> - load argument pointer with size
 *
 * expression:
 *     ldargp <argument_index>
 * operation:
 *     arg = GetArgumentEntry(argument_index)
 *     PushN(arg.valueref)
 *     PushN(arg.size)
 * stack changes:
 *     ... -> ..., arg.valueref, arg.size
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 *     #INV (invalid argument_index is specified)
 */
DEFINE_INST_O1	(Ldargp,		ldargp,				Imm16	)

/*
 * ldpv.<type> - load value from pointer
 *
 * expression:
 *     ldpv.<x1|x2|x4|x8>
 * operation:
 *     ref = PopN()
 *     value = ReadT(type, ref)
 *     PushN(value)
 * stack changes:
 *     ..., ref -> ..., value
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 *     #ACC (invalid ref is specified)
 */
DEFINE_INST		(Ldpv_X1,		ldpv.x1				)
DEFINE_INST		(Ldpv_X2,		ldpv.x2				)
DEFINE_INST		(Ldpv_X4,		ldpv.x4				)
DEFINE_INST		(Ldpv_X8,		ldpv.x8				)

/*
 * stpv.<type> - store value to pointer
 *
 * expression:
 *     stpv.<x1|x2|x4|x8>
 * operation:
 *     value = PopT(type)
 *     ref = PopN()
 *     WriteT(type, ref, value)
 * stack changes:
 *     ..., ref, value -> ...
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 *     #ACC (invalid ref is specified)
 */
DEFINE_INST		(Stpv_X1,		stpv.x1				)
DEFINE_INST		(Stpv_X2,		stpv.x2				)
DEFINE_INST		(Stpv_X4,		stpv.x4				)
DEFINE_INST		(Stpv_X8,		stpv.x8				)

/*
 * ppcpy - pointer to pointer copy
 *
 * expression:
 *     ppcpy
 * operation:
 *     size = PopN()
 *     source_address = PopN()
 *     destination_address = PopN()
 *     Copy(destination_address, source_address, size)
 * stack changes:
 *     ..., destination_address, source_address, size -> ...
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 *     #ACC (invalid target address is specified)
 */
DEFINE_INST		(Ppcpy,			ppcpy				)

/*
 * pvfil.<type> - fill address range with value
 *
 * expression:
 *     pvfil.<x1|x2|x4|x8>
 * operation:
 *     count = PopN()
 *     value = PopT(type)
 *     destination_address = PopN()
 *     FillT(type, destination_address, value, count)
 * stack changes:
 *     ..., destination_address, value, count -> ...
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 *     #ACC (invalid target address is specified)
 */
DEFINE_INST		(Pvfil_X1,		pvfil.x1			)
DEFINE_INST		(Pvfil_X2,		pvfil.x2			)
DEFINE_INST		(Pvfil_X4,		pvfil.x4			)
DEFINE_INST		(Pvfil_X8,		pvfil.x8			)

/*
 * Initarg - initialize argument table
 *
 * expression:
 *     initarg
 * operation:
 *     ATP = SSP.ShadowFrame.ATPBASE // get AT base from shadow stack
 *     XTS.ATE = 0 // clear AT enabled bit
 * stack changes:
 *     none
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 */
DEFINE_INST		(Initarg,		initarg				)

/*
 * arg <size> - add argument entry with size
 *
 * expression:
 *     arg <size>
 * operation:
 *     if (size == 0 || size > MaximumSizeSingleArgument) {
 *         RaiseException(#INV)
 *         End
 *     }
 *     if (SSP.ShadowFrame.SP < SP + size) {
 *         RaiseException(#INV)
 *         End
 *     }
 *     argument_index = GetArgumentCount()
 *     AddArgumentEntry(argument_index, SP, size) // XTS.ATE is automatically set
 * stack changes:
 *     none
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 *     #INV (maximum argument count exceeded)
 *          (invalid size)
 */
DEFINE_INST_O1	(Arg,			arg,				Imm32	)

/*
 * var <size> - add local variable entry with size
 *
 * expression:
 *     var <size>
 * operation:
 *     if (size == 0) {
 *         RaiseException(#ACC)
 *         End
 *     }
 *     local_variable_index = GetLocalVariableCount()
 *     AddLocalVariableEntry(local_variable_index, SP, size)
 * stack changes:
 *     none
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 *     #ACC (maximum local variable count exceeded)
 *          (invalid size)
 */
DEFINE_INST_O1	(Var,			var,				Imm32	)

/*
 * dcv - discard 1 value in stack
 *
 * expression:
 *     dcv
 * operation:
 *     value = PopN()
 * stack changes:
 *     ..., value -> ...
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 */
DEFINE_INST		(Dcv,			dcv					)

/*
 * dcvn - discard values in stack
 *
 * expression:
 *     dcvn
 * operation:
 *     n = PopN()
 *     for (i = 0; i < n; i++) {
 *         PopN()
 *     }
 * stack changes:
 *     ..., value_1, value_2, ..., value_n, n -> ...
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 */
DEFINE_INST		(Dcvn,			dcvn				)

/*
 * test_<condition>.<type> - test condition
 *
 * expression:
 *     test_<e|z>.<i4|i8|f4|f8> (test v1 == v2)
 *     test_<ne|nz>.<i4|i8|f4|f8> (test v1 != v2)
 *     test_<le|ng>.<i4|i8|u4|u8|f4|f8> (test v1 <= v2)
 *     test_<ge|nl>.<i4|i8|u4|u8|f4|f8> (test v1 >= v2)
 *     test_<l|nge>.<i4|i8|u4|u8|f4|f8> (test v1 < v2)
 *     test_<g|nle>.<i4|i8|u4|u8|f4|f8> (test v1 > v2)
 * operation:
 *     v2 = PopT(type)
 *     v1 = PopT(type)
 *     test condition for v1, v2
 *     if (condition was true)
 *         result = 1
 *     else
 *         result = 0
 *     PushN(result)
 * stack changes:
 *     ..., v1, v2 -> ..., result
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 */
DEFINE_INST		(Test_e_I4,     test_e.i4           )
DEFINE_INST		(Test_e_I8,     test_e.i8           )
DEFINE_INST		(Test_e_F4,     test_e.f4           )
DEFINE_INST		(Test_e_F8,     test_e.f8           )
DEFINE_INST		(Test_ne_I4,    test_ne.i4          )
DEFINE_INST		(Test_ne_I8,    test_ne.i8          )
DEFINE_INST		(Test_ne_F4,    test_ne.f4          )
DEFINE_INST		(Test_ne_F8,    test_ne.f8          )
DEFINE_INST		(Test_le_I4,    test_le.i4          )
DEFINE_INST		(Test_le_I8,    test_le.i8          )
DEFINE_INST		(Test_le_U4,    test_le.u4          )
DEFINE_INST		(Test_le_U8,    test_le.u8          )
DEFINE_INST		(Test_le_F4,    test_le.f4          )
DEFINE_INST		(Test_le_F8,    test_le.f8          )
DEFINE_INST		(Test_ge_I4,    test_ge.i4          )
DEFINE_INST		(Test_ge_I8,    test_ge.i8          )
DEFINE_INST		(Test_ge_U4,    test_ge.u4          )
DEFINE_INST		(Test_ge_U8,    test_ge.u8          )
DEFINE_INST		(Test_ge_F4,    test_ge.f4          )
DEFINE_INST		(Test_ge_F8,    test_ge.f8          )
DEFINE_INST		(Test_l_I4,     test_l.i4           )
DEFINE_INST		(Test_l_I8,     test_l.i8           )
DEFINE_INST		(Test_l_U4,     test_l.u4           )
DEFINE_INST		(Test_l_U8,     test_l.u8           )
DEFINE_INST		(Test_l_F4,     test_l.f4           )
DEFINE_INST		(Test_l_F8,     test_l.f8           )
DEFINE_INST		(Test_g_I4,     test_g.i4           )
DEFINE_INST		(Test_g_I8,     test_g.i8           )
DEFINE_INST		(Test_g_U4,     test_g.u4           )
DEFINE_INST		(Test_g_U8,     test_g.u8           )
DEFINE_INST		(Test_g_F4,     test_g.f4           )
DEFINE_INST		(Test_g_F8,     test_g.f8           )

/*
 * br.<type> <relative_offset> - unconditional branch
 *
 * expression:
 *     br.<i1|i2|i4> <relative_offset>
 * operation:
 *     PC = GetNextPC() + SignExtendN(relative_offset)
 * stack changes:
 *     none
 * exceptions:
 *     #ACC (invalid relative_offset is specified)
 */
DEFINE_INST_O1	(Br_I1,			br.i1,				Imm8	)
DEFINE_INST_O1	(Br_I2,			br.i2,				Imm16	)
DEFINE_INST_O1	(Br_I4,			br.i4,				Imm32	)

/*
 * br_<condition>.<type> <relative_offset> - conditional branch
 *
 * expression:
 *     br_z.<i1|i2|i4> <relative_offset> (branch if test == 0)
 *     br_nz.<i1|i2|i4> <relative_offset> (branch if test != 0)
 * operation:
 *     test = PopN()
 *     check condition with test
 *     if (condition is met) {
 *         PC = GetNextPC() + SignExtendN(relative_offset)
 *     }
 * stack changes:
 *     ..., test -> ...
 * exceptions:
 *     #ACC (invalid relative_offset is specified)
 */
DEFINE_INST_O1	(Br_z_I1,		br_z.i1,			Imm8	)
DEFINE_INST_O1	(Br_z_I2,		br_z.i2,			Imm16	)
DEFINE_INST_O1	(Br_z_I4,		br_z.i4,			Imm32	)
DEFINE_INST_O1	(Br_nz_I1,		br_nz.i1,			Imm8	)
DEFINE_INST_O1	(Br_nz_I2,		br_nz.i2,			Imm16	)
DEFINE_INST_O1	(Br_nz_I4,		br_nz.i4,			Imm32	)

/*
 * call.<type> <relative_offset> - call function
 *
 * expression:
 *     call.<i1|i2|i4> <relative_offset>
 * operation:
 *     return_address = GetNextPC()
 *     PushN(return_address) // save return address
 *     PC = return_address + SignExtendN(relative_offset)
 * 
 *     // build shadow frame (return address, SP, LVT, AT, LVT/AT state)
 *     ShadowPushN(return_address)
 *     ShadowPushN(SP)
 *     ShadowPushN(LVTP)
 *     ShadowPushN(0)
 *     ShadowPushN(0) // LVT/AT state
 * stack changes:
 *     ... -> ..., return_address (stack)
 *     ... -> ..., return_address_prev, SP_prev, LVT_prev, AT_prev, LVT_AT_prev_state (shadow stack)
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 *     #ACC (invalid relative_offset is specified)
 */
DEFINE_INST_O1	(Call_I1,		call.i1,			Imm8	)
DEFINE_INST_O1	(Call_I2,		call.i2,			Imm16	)
DEFINE_INST_O1	(Call_I4,		call.i4,			Imm32	)

/*
 * ret - return to previous function
 *
 * expression:
 *     ret
 * operation:
 *     // pop shadow frame
 *     LVT_AT_prev_state = ShadowPopN()
 *     AT_prev = ShadowPopN()
 *     LVT_prev = ShadowPopN()
 *     SP_prev = ShadowPopN()
 *     return_address_prev = ShadowPopN()
 * 
 *     // pop return address
 *     return_address = PopN()
 * 
 *     if (return_address != return_address_prev ||
 *         SP != SP_prev) {
 *         RaiseException(#ACC)
 *         End
 *     }
 *
 *     PC = return_address
 *     // FIXME: should we do something with LVT or AT?
 * stack changes:
 *     ..., return_address -> ... (stack)
 *     ..., return_address_prev, SP_prev, LVT_prev, AT_prev, LVT_AT_prev_state -> ... (shadow stack)
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 *     #ACC (invalid return_address is specified)
 */
DEFINE_INST		(Ret,			ret					)

/*
 * nop - no operation
 *
 * expression:
 *     nop
 * operation:
 *     do nothing
 * stack changes:
 *     none
 * exceptions:
 *     none
 */
DEFINE_INST		(Nop,			nop					)

/*
 * bp - breakpoint
 *
 * expression:
 *     nop
 * operation:
 *     RaiseException(#BP)
 * stack changes:
 *     none
 * exceptions:
 *     #BP (breakpoint detected)
 */
DEFINE_INST		(Bp,			bp					)

/*
 * ldvmsr <vmsr_index> - load virtual machine special register
 *
 * expression:
 *     ldvmsr <vmsr_index>
 * operation:
 *     vmsr_value = GetVMSR_N(vmsr_index)
 *     PushN(vmsr_value) // VMSR[vmsr_index]
 * stack changes:
 *     ... -> ..., vmsr_value
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 *     #INV (invalid vmsr_index is specified)
 */
DEFINE_INST_O1	(Ldvmsr,		ldvmsr,				Imm16	)

/*
 * stvmsr <vmsr_index> - store virtual machine special register
 *
 * expression:
 *     stvmsr <vmsr_index>
 * operation:
 *     vmsr_value = PopN()
 *     SetVMSR_N(vmsr_index, vmsr_value) // VMSR[vmsr_index] = vmsr_value
 * stack changes:
 *     ..., vmsr_value -> ...
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 *     #INV (invalid vmsr_index is specified)
 *     #ACC (VMSR[vmsr_index] is read-only)
 */
DEFINE_INST_O1	(Stvmsr,		stvmsr,				Imm16	)

/*
 * vmcall <vmcall_identifier> - call virtual machine service
 *
 * expression:
 *     vmcall <vmcall_identifier>
 * operation:
 *     result = VMCall(vmcall_identifier)
 *     PushN(result)
 *     // FIXME: TBD
 * stack changes:
 *     ... -> ..., result
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 *     #INV (invalid vmcall_identifier is specified)
 */
DEFINE_INST_O1	(Vmcall,		vmcall,				Imm32	)

/*
 * vmxthrow <identifier> - throw exception to virtual machine
 *
 * expression:
 *     vmxthrow <identifier>
 * operation:
 *     param4 = PopN()
 *     param3 = PopN()
 *     param2 = PopN()
 *     param1 = PopN()
 *     result = RaiseException(identifier, param1, param2, param3, param4)
 *     PushN(result)
 *     // FIXME: TBD
 * stack changes:
 *     ..., param1, param2, param3, param4 -> ..., result
 * exceptions:
 *     #STK (stack overflows/underflows during execution)
 *     #INV (invalid identifier is specified)
 */
DEFINE_INST		(Vmxthrow,		vmxthrow			)




/*
call (Clear LVT)
-> var, var, ... (Build temporary LVT)
-> initsf (Create LVT from temporary LVT)
-> ...
-> ret (Delete newly created LVT)

initarg, stargt, stargt, ... (Build temporary AT)
-> call (Create AT from temporary AT)
-> ...
-> ret (Delete newly created AT)

	initarg		; initialize argument table (optional)

	ldimm.i4 $hello_world_string
	arg 4		; arg #0, size 4
	ldimm.i1 13
	arg 4		; arg #1, size 4

	call $print ; implicitly build shadow frame

	ret

print:
	var 4       ; local var #0, size 4
	var 4       ; local var #1, size 4

    ; no need to create (or delete) stack frame explicitly

	...

	ret

*/

