

#include <stdio.h>
#include <windows.h>
#include <cstdint>
#include <map>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <atomic>
#include <iostream>


#pragma comment(lib, "../CoreStaticLib.lib")

#include "../CoreStaticLib/svm/vmbase.h"
#include "../CoreStaticLib/svm/vmmemory.h"
#include "../CoreStaticLib/svm/bc_emitter.h"
#include "../CoreStaticLib/svm/bc_interpreter.h"
#include "../CoreStaticLib/svm/integer.h"

using namespace VM_NAMESPACE;

void test_bitmap()
{
    uint8_t bits_1[] = { 0xff, 0xff, 0xff, 0xff };
    uint8_t bits_2[] = { 0, 0, 0, 0 };
    uint8_t bits_3[] = { 0xff, 0xff, 0xff, 0xff };
    uint8_t bits_4[] = { 0, 0, 0, 0 };
    Bitmap b1(std::size(bits_1) << 3, bits_1);
    Bitmap b2(std::size(bits_2) << 3, bits_2);
    Bitmap b3(std::size(bits_3) << 3, bits_3);
    Bitmap b4(std::size(bits_4) << 3, bits_4);

    b1.Clear(1);
    b1.Set(1);

    b1.ClearRange(1, 3);
    b1.ClearRange(8 + 2, 11);
    b2.SetRange(1, 3);
    b2.SetRange(8 + 2, 11);

    bits_1[0] = 0x10;

    size_t r[] =
    {
        b1.FindFirstClear(4),
        b1.FindFirstSet(4),
        b1.FindLastClear(11),
        b1.FindLastSet(11),

        b1.FindFirstClear(11),
        b1.FindFirstSet(11),
        b1.FindLastClear(4),
        b1.FindLastSet(4),

        b3.FindFirstClear(0),
        b4.FindFirstSet(0),
        b3.FindLastClear(b3.Count() - 1),
        b4.FindLastSet(b4.Count() - 1),
    };

    __nop();
}

void test_memory_manager()
{
    VMMemoryManager Mm(0x7f00000);
    VMMemoryManager Mm2(0x7f00000);
    VMMemoryManager Mm3(0x7f00000);
    VMMemoryManager Mm4(0x100000);
    uint64_t ResultAddress = 0;

    uint32_t Option = VMMemoryManager::Options::UsePreferredAddress;

    Mm.Allocate(0, 0x100000, MemoryType::Stack, 0x1234, Option, ResultAddress);
    Mm2.Allocate(0x7f00000 - 0x2000, 0x2000, MemoryType::Stack, 0xffff, Option, ResultAddress);
    Mm3.Allocate(0xd000, 0x3000, MemoryType::Stack, 0xdeadbeef, Option, ResultAddress);

    Mm4.Allocate(0x10000, 0x1000, MemoryType::Stack, 0xdeadbeef, Option, ResultAddress);
    Mm4.Allocate(0x30000, 0x1000, MemoryType::Stack, 0xdeadbeef, Option, ResultAddress);
    Mm4.Allocate(0x50000, 0x1000, MemoryType::Stack, 0xdeadbeef, Option, ResultAddress);
    Mm4.Allocate(0x70000, 0x1000, MemoryType::Stack, 0xdeadbeef, Option, ResultAddress);

    Mm4.Allocate(0, 0x20000, MemoryType::Stack, 0xbaba, 0, ResultAddress);

    Mm4.Free(ResultAddress + 0x7000, 0);
    Mm4.Free(ResultAddress + 0x4000, 12);
    Mm4.Free(ResultAddress, 0);

    Mm4.Free(0x30000, 0);

    Mm4.Allocate(0x1000, 0x10000, MemoryType::Stack, 0xdeadbeef, Option, ResultAddress);
    Mm4.Allocate(0x1000, 0x20000, MemoryType::Stack, 0xdeadbeef, Option, ResultAddress);
    Mm4.Allocate(0x1000, 0x30000, MemoryType::Stack, 0xdeadbeef, Option, ResultAddress);
    Mm4.Allocate(0x1000, 0x40000, MemoryType::Stack, 0xdeadbeef, Option, ResultAddress);


    uint8_t wb[] = { '1', '2', '3', '4' };
    Mm.Write(0xffe, 4, wb);

    Mm.Free(0, 0x111);

}

void test_vm_stack()
{
    size_t StackSize = 0x1000;
    uint64_t ResultAddress = 0;
    VMMemoryManager MmStack(StackSize);

    DASSERT(MmStack.Allocate(0, StackSize, MemoryType::Stack, 0, 0, ResultAddress));
    uintptr_t StackBase = ResultAddress + MmStack.Base();
    MmStack.Write(ResultAddress, 1, reinterpret_cast<uint8_t*>("!"));

//	VMStack Stack(StackBase, StackSize, sizeof(intptr_t));

    unsigned char TempBuffer[0x100];
    VMStack Stack(reinterpret_cast<uint64_t>(TempBuffer), sizeof(TempBuffer), 4);

    std::string TestString("Hello world!");
    DASSERT(Stack.Push(0x1234));
    DASSERT(Stack.Push(0xdeadbeefbaadf00d));
    DASSERT(Stack.Push(reinterpret_cast<uint8_t*>(const_cast<char*>(TestString.c_str())), TestString.length()));

    uint32_t t1 = 0;
    uint64_t t2 = 0;
    uint8_t Buffer[0x40]{};

    DASSERT(Stack.Pop(Buffer, TestString.length()));
    DASSERT(Stack.Pop(&t2));
    DASSERT(Stack.Pop(&t1));

    DASSERT(Stack.Pop(&t1)); // must boom here!
}

void test_int64x64()
{
#if M_IS_ARCH_64
    unsigned char rand_bytes[16];

    srand(static_cast<unsigned int>(__rdtsc()));

    for (int i = 0; 1; i++)
    {
        for (auto& it : rand_bytes)
            it = rand();

        uint64_t v1 = Base::FromBytes<uint64_t>(rand_bytes);
        uint64_t v2 = Base::FromBytes<uint64_t>(rand_bytes + 8);

        int64_t res1_lo, res1_hi;
        int64_t res2_lo, res2_hi;
        res1_lo = Base::Int64x64To128(v1, v2, &res1_hi);
        res2_lo = _mul128(v1, v2, &res2_hi);

        if (!IsDebuggerPresent() ||
            IsDebuggerPresent() && !(GetTickCount() % 1000))
        {
            printf("Test %02d %016llx x %016llx:\n"
                "-> %-8s %016llx`%016llx\n"
                "-> %-8s %016llx`%016llx\n\n",
                i, v1, v2,
                "mine", res1_hi, res1_lo,
                "vs2015", res2_hi, res2_lo);
        }

        if (res1_lo != res2_lo || res1_hi != res2_hi)
        {
            printf("result mismatch! stop.\n");
            break;
        }
    }
#endif
}

void test_vm_bytecode_emitter()
{
    struct AllocateParams
    {
        uint64_t PreferredAddress;
        size_t Size;
        MemoryType Type;
        intptr_t Tag;
        uint32_t Options;
        uint64_t ResultAddress;
        const char* Description;
    };

    AllocateParams AllocParamTable[] =
    {
        { 0x00001000, 0x0000f000, MemoryType::Bytecode, 0, VMMemoryManager::Options::UsePreferredAddress, 0, "GuestCode" },

        { 0x00000000, 0x00010000, MemoryType::Stack, 0, 0, 0, "GuestStack" }, 
        { 0x00000000, 0x00010000, MemoryType::Stack, 0, 0, 0, "GuestShadowStack" }, 
        { 0x00000000, 0x00010000, MemoryType::Stack, 0, 0, 0, "GuestLocalVarStack" }, 
        { 0x00000000, 0x00010000, MemoryType::Stack, 0, 0, 0, "GuestArgumentStack" }, 
    };

    AllocateParams& GuestCode = AllocParamTable[0];
    AllocateParams& GuestStack = AllocParamTable[1];
    AllocateParams& GuestShadowStack = AllocParamTable[2];
    AllocateParams& GuestLocalVarStack = AllocParamTable[3];
    AllocateParams& GuestArgumentStack = AllocParamTable[4];


    VMMemoryManager Memory(0x4000000);

    for (auto& it : AllocParamTable)
    {
        if (!Memory.Allocate(it.PreferredAddress, it.Size, it.Type, it.Tag, it.Options, it.ResultAddress))
        {
            printf("failed to allocate guest memory for %s\n", it.Description);
            return;
        }

        // touch
        printf("touching guest memory 0x%016llx - 0x%016llx (%s)\n",
            it.ResultAddress, it.ResultAddress + it.Size - 1, it.Description);
        Memory.Fill(it.ResultAddress, it.Size, 0xdd);
    }

    VMBytecodeEmitter Emitter;
    unsigned char *HostAddress = reinterpret_cast<unsigned char *>
        (Memory.HostAddress(GuestCode.ResultAddress, 0x1000));
    size_t ResultSize = 0;

    bool Result = Emitter
        .BeginEmit()
        .Emit(Opcode::T::Ldimm_I1, Operand(OperandType::Imm8, 0xf1))
        .Emit(Opcode::T::Ldimm_I2, Operand(OperandType::Imm16, 0xf123))
        .Emit(Opcode::T::Ldimm_I4, Operand(OperandType::Imm32, 0xf1234567))
        .Emit(Opcode::T::Ldimm_I8, Operand(OperandType::Imm64, 0xf123456789abcdef))
        .Emit(Opcode::T::Ldimm_I1, 4)
        .Emit(Opcode::T::Dcvn)
        .Emit(Opcode::T::Ldimm_I4, Operand(OperandType::Imm32, 1))
        .Emit(Opcode::T::Ldimm_I4, Operand(OperandType::Imm32, 2))
        .Emit(Opcode::T::Ldimm_I4, Operand(OperandType::Imm32, 3))
        .Emit(Opcode::T::Add_I4)
        .Emit(Opcode::T::Add_I4)
        .Emit(Opcode::T::Bp)
        .EndEmit(HostAddress, GuestCode.Size, &ResultSize);

    printf("EmitResult %d, size %zd\n", Result, ResultSize);

    const int DefaultAlignment = sizeof(intptr_t);

    VMExecutionContext ExecutionContext{};
    ExecutionContext.IP = static_cast<uint32_t>(GuestCode.ResultAddress);
    ExecutionContext.XTableState = 0;
    ExecutionContext.ExceptionState = ExceptionState::T::None;
    ExecutionContext.Stack = VMStack(
        Memory.HostAddress(GuestStack.ResultAddress), GuestStack.Size, DefaultAlignment);
    ExecutionContext.ShadowStack = VMStack(
        Memory.HostAddress(GuestShadowStack.ResultAddress), GuestShadowStack.Size, DefaultAlignment);

    ExecutionContext.LocalVariableStack = VMStack(
        Memory.HostAddress(GuestLocalVarStack.ResultAddress), GuestLocalVarStack.Size, DefaultAlignment);
    ExecutionContext.ArgumentStack = VMStack(
        Memory.HostAddress(GuestArgumentStack.ResultAddress), GuestArgumentStack.Size, DefaultAlignment);

    VMBytecodeInterpreter Interpreter(Memory);
    Interpreter.Execute(ExecutionContext, 9999999);

    [] {}();
}

template <
    typename T,
    std::enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value, bool> = true>
    class signed_test_int // signed
{
public:
    signed_test_int(T val = 0) : val_(val) {}

    T val() const
    {
        return val_;
    }

    void print() const
    {
        printf("val = %d\n", val_);
    }

    T val_;
};

template <
    typename T,
    std::enable_if_t<std::is_integral<T>::value&& std::is_unsigned<T>::value, bool> = true>
    class unsigned_test_int // unsigned
{
public:
    unsigned_test_int(T val = 0) : val_(val) {}

    T val() const
    {
        return val_;
    }

    void print() const
    {
        printf("val = %d\n", val_);
    }

    T val_;
};


template <
    typename T,
    typename = std::enable_if_t<std::is_integral<T>::value, T>>
    class real_test_int // default
{
public:
    void test()
    {
        printf("default\n");
    }
};

template <typename T>
class real_test_int<T, typename std::enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value, T>>
{
public:
    void test()
    {
        printf("signed int specialization\n");
    }
};

template <typename T>
class real_test_int<T, typename std::enable_if_t<std::is_integral<T>::value&& std::is_unsigned<T>::value, T>>
{
public:
    void test()
    {
        printf("unsigned int specialization\n");
    }
};


void test_integer()
{
    using TInt = typename uint8_t;
    Integer<TInt> nan;           // nan
    Integer<TInt> a = 12;
    Integer<TInt> b = a + 34;    // ok, 12 + 34 = 46
    Integer<TInt> c = b * 2;     // ok, 92
    Integer<TInt> d = b * 123;   // ok, 0x161a with overflow => 0x1a = 26
    Integer<TInt> e = b / 0;     // nan (div/0)
    Integer<TInt> f = b % 0;     // nan (div/0)
    Integer<TInt> g = b & 0x0f;  // ok, b and 0x0f = 0x2e and 0x0f = 14
    Integer<TInt> h = b | 0xf0;  // ok, b or 0xf0 = 0x2e or 0xf0 = 0xfe = 254
    Integer<TInt> i = b ^ 0xff;  // ok, b xor 0xff = 0x2e xor 0xff = not 0x2e = 0xd1 = 209
    Integer<TInt> j = b << 1;    // ok, b shl 1 = 0x2e shl 1 = 0x5c = 92
    Integer<TInt> k = b << 4;    // ok, b shl 4 = 0x2e shl 4 = 0x2e0 with overflow => 0xe0 = 224
    Integer<TInt> l = b << 8;    // ok, overflow => 0
    Integer<TInt> m = b << 123;  // ok, overflow => 0
    Integer<TInt> o = b >> 1;    // ok, b shr 1 = 0x2e shr 1 = 0x17 = 23
    Integer<TInt> p = b >> 4;    // ok, b shr 4 = 0x2e shr 4 = 0x02 = 2
    Integer<TInt> q = b >> 8;    // ok, 0
    Integer<TInt> r = b >> 123;  // ok, 0
    Integer<TInt> s = ~b;        // ok, not 0x2e = 0xd1 = 209
    Integer<TInt> t = -b;        // ok, negate 0x2e = not 0x2e + 1 = 0xd2 = 210

    Integer<TInt> nan2 = h + nan;    // nan

    auto print = [](const auto& i, const char *s)
    {
        using TClass = std::remove_reference_t<decltype(i)>;

        std::string flags;
        if (i.State() & TClass::StateFlags::DivideByZero)
        {
            if (flags.length() > 0)
                flags.append(" | ");
            flags.append("Div/0");
        }
        if (i.State() & TClass::StateFlags::Overflow)
        {
            if (flags.length() > 0)
                flags.append(" | ");
            flags.append("Overflow");
        }

        std::cout
            << "integer " << s << " -> "
            << "is_nan: " << i.Invalid() << ", "
            << "val: " << +i.Value() << ", "
            << "flags: [ " << flags << " ]" << std::endl;
    };

    #define int_param(_s)     (_s), #_s

    print(int_param(nan));
    print(int_param(a));
    print(int_param(b));
    print(int_param(c));
    print(int_param(d));
    print(int_param(e));
    print(int_param(f));
    print(int_param(g));
    print(int_param(h));
    print(int_param(i));
    print(int_param(j));
    print(int_param(k));
    print(int_param(l));
    print(int_param(m));
    print(int_param(o));
    print(int_param(p));
    print(int_param(q));
    print(int_param(r));
    print(int_param(s));
    print(int_param(t));
    print(int_param(nan2));

    #undef int_param
}

int main()
{
    test_integer();

    return 0;


    test_vm_bytecode_emitter();


    return 0;

    ::SetWindowPos(GetConsoleWindow(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

    return 0;
}


