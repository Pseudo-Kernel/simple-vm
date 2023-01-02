
#pragma once

#include <cstdint>
#include <map>
#include <vector>
#include <memory>
#include <functional>
#include <type_traits>
#include <string>
#include <algorithm>

#define DASSERT(_x)          if (!(_x)) { __debugbreak(); }

#define VM_NAMESPACE            VM

// Annotations
#define threadsafe

#include "base.h"
#include "utility.h"

#include "vminst.h"

namespace VM_NAMESPACE
{

    namespace Constants
    {
        constexpr const static size_t MaximumSizeSingleArgument = 0x400000;
        constexpr const static size_t MaximumFunctionArgumentCount = 0x40;

        constexpr const static size_t MaximumSizeSingleLocalVariable = 0x400000;
        constexpr const static size_t MaximumFunctionLocalVariableCount = 0x40;
    };


    struct LocalVariableTableEntry
    {
        uint32_t Size;
        uint32_t Address;
    };

    struct ArgumentTableEntry
    {
        uint32_t Size;
        uint32_t Address;
    };

    struct ShadowFrame
    {
        //
        // Shadow Frame Structure.
        // 
        // |      ...       |
        // +----------------+----+
        // | return address |    |
        // +----------------+    |
        // | prev SP        |    |
        // +----------------+    |
        // | LVT address    |  Frame
        // +----------------+    |
        // | AT address     |    |
        // +----------------+    |
        // | prev XT state  |    |
        // +----------------+----+
        // |      ...       |
        // 
        //

        uint32_t XTableState;			// Previous LVT/AT State
        uint32_t ATP;			// Argument Table (in current function)
        uint32_t LVTP;			// Local Variable Table (in current function)
        uint32_t ReturnSP;		// Previous SP
        uint32_t ReturnIP;		// Previous IP
    };

    static_assert(
        std::is_standard_layout<ShadowFrame>::value,
        "struct is not standard layout");

    struct XTableStateBits
    {
        enum T : uint32_t
        {
            ArgumentTableReady = 1 << 0,
            LocalVariableTableReady = 1 << 1,
        };
    };

    struct ExceptionState
    {
        enum T : uint32_t
        {
            None,

            StackOverflow,
            InvalidInstruction,
            InvalidAccess,
            IntegerDivideByZero,
            Breakpoint,
            SingleStep,

            FloatingPointInvalid,   // TBD
            IntegerOverflow,        // TBD
        };
    };

    struct InstructionPrefixBits
    {
        enum T : uint32_t
        {
            None = 0,

            CheckOverflow = 1 << 0,     // Raise exception if result overflows (integer)
        };
    };
    
    struct ModeBits
    {
        enum T : uint32_t
        {
            VMStackOper64Bit = 1 << 0,	// default = 32-bit stack operation (if not set)
            VMPointer64Bit = 1 << 1,	// default = 32-bit pointer (if not set)
        };
    };

    struct VMExecutionContext
    {
        alignas(16) uint32_t Lock;
        uint32_t Reserved1[15];					// Padding

        uint32_t IP;							// Instruction Pointer
        uint32_t XTableState;					// XTS (LVT/AT State)

        VMStack Stack;				// Holds SP, SPSTART, SPEND

        //
        // Shadow Stack.
        // See ShadowFrame
        //

        VMStack ShadowStack;			// Holds SSP, SSPSTART, SSPEND

        // 
        // Local Variable Stack & Argument Stack.
        // 

        VMStack LocalVariableStack;	// Holds LVTP, LVTPSTART, LVTPEND
        VMStack ArgumentStack;		// Holds ATP, ATPSTART, ATPEND

        //
        // Exception State.
        //

        //uint32_t XCE;							// Exception Condition (TBD)
        uint32_t ExceptionState;				// Exception Status. see ExceptionState::T.

        //
        // Temporary State.
        //

        uint32_t NextIP;						// Temporary IP
        uint32_t FetchedPrefix;                 // Fetched Prefix Bits. see InstructionPrefixBits::T.

        // 
        // Virtual Machine Specific Register (VMSR).
        // 

        uint32_t VMSR[32];

        //
        // Special Mode Register.
        //

        uint32_t Mode;							// Mode bits. See ModeBits::T.
    };

    static_assert(
        std::is_standard_layout<VMExecutionContext>::value &&
        std::is_trivially_copyable<VMExecutionContext>::value,
        "class type is not a standard layout type");



}

