#include "stdafx.h"
#include "unittest_helper.h"
#include "CppUnitTest.h"

#include <stdarg.h>
#include <conio.h>
#include <windows.h>
#include "../CoreStaticLib/svm/arch.h"
#include "../CoreStaticLib/svm/base.h"
#include "../CoreStaticLib/svm/vmbase.h"
#include "../CoreStaticLib/svm/vmmemory.h"
#include "../CoreStaticLib/svm/integer.h"
#include "../CoreStaticLib/svm/bc_interpreter.h"
#include "../CoreStaticLib/svm/bc_emitter.h"

#pragma comment(lib, "../CoreStaticLib.lib")

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace VM_NAMESPACE;

constexpr uint64_t PtrToU64(void *Pointer)
{
    return static_cast<uint64_t>(reinterpret_cast<size_t>(Pointer));
}

template <
    typename T,
    typename = std::enable_if_t<std::is_integral<T>::value>>
T Random(bool SetMsb)
{
    constexpr const auto Size = sizeof(T);
    alignas(T) unsigned char RandArray[Size]{};

    for (auto& e : RandArray)
        e = static_cast<unsigned char>(rand());

    using UnsignedType = std::make_unsigned_t<T>;
    UnsignedType Result = Base::FromBytes(RandArray);

    constexpr UnsignedType Msb = 1 << ((Size << 3) - 1);
    if (SetMsb)
    {
        Result |= Msb;
    }
    else
    {
        Result &= ~Msb;
    }

    return static_cast<T>(Result);
}

template <
    typename T,
    typename = std::enable_if_t<std::is_integral<T>::value>>
T Sign(T Value, bool SetSign = true)
{
    using UnsignedType = std::make_unsigned_t<T>;
    constexpr const UnsignedType SignBit = 
        static_cast<UnsignedType>(1) << ((sizeof(T) << 3) - 1);
    UnsignedType Result = Value;

    if (SetSign)
        Result |= SignBit;
    else
        Result &= ~SignBit;

    return Result;
}

std::string Format(const char* Format, ...)
{
    char Buffer[512];
    va_list args;
    va_start(args, Format);
    vsprintf_s(Buffer, Format, args);
    va_end(args);

    return std::string(Buffer);
}

std::wstring Format(const wchar_t *Format, ...)
{
    wchar_t Buffer[512];
    va_list args;
    va_start(args, Format);
    vswprintf_s(Buffer, Format, args);
    va_end(args);

    return std::wstring(Buffer);
}

namespace CoreUnitTest
{
    TEST_CLASS(VMStackTest)
    {
    public:
        TEST_METHOD(Stack_MasterTest)
        {
            unsigned char StackBytes[64]{};
            constexpr const size_t StackSize = sizeof(StackBytes);
            std::memset(StackBytes, 0xdd, StackSize);

            if (1)
            {
                int StackAlignment = 4;
                VMStack Stack(PtrToU64(StackBytes), StackSize, StackAlignment);
                Stack_Testcases(Stack);
            }

            if (1)
            {
                int StackAlignment = 8;
                VMStack Stack(PtrToU64(StackBytes), StackSize, StackAlignment);
                Stack_Testcases(Stack);
            }
        }

    private:
        template <
            typename TPush,
            typename TPop = TPush,
            typename = std::enable_if_t<
            std::is_integral<TPush>::value&& std::is_integral<TPop>::value>>
            void Stack_TestHelper_PushPop(VMStack& Stack, const TPush& PushValue, TPop PopValueExpected)
        {
            constexpr const uint32_t PushTypeSize = sizeof(TPush);
            constexpr const uint32_t PopTypeSize = sizeof(TPop);
            uint32_t Alignment = Stack.Alignment();
            uint32_t AlignmentMask = Stack.Alignment() - 1;
            uint32_t ExpectedDecrementSize = (PushTypeSize + AlignmentMask) & ~AlignmentMask;

            Logger::WriteMessage(__FUNCSIG__);
            Logger::WriteMessage(Format(
                L"push/pop test, stack alignment %d, push_size %d, pop_size %d",
                Alignment, PushTypeSize, PopTypeSize).c_str());

            // Push test
            uint32_t OffsetBeforePush = Stack.TopOffset();
            Assert::IsTrue(
                Stack.Push<TPush>(PushValue),
                L"push: push failed");
            Assert::AreEqual(
                OffsetBeforePush - ExpectedDecrementSize,
                Stack.TopOffset(),
                L"push: unexpected stack top offset");

            // Test whether the bytes are actually copied
            unsigned char* p = reinterpret_cast<unsigned char*>(Stack.Top());
            unsigned char Buffer[PushTypeSize]{};
            Base::ToBytes(PushValue, Buffer);
            Assert::IsTrue(
                !std::memcmp(Buffer, p, PushTypeSize),
                L"push: pushed value is not same");

            // Pop test
            if (Alignment >= PushTypeSize &&
                Alignment >= PopTypeSize)
            {
                // 1 element
                TPop PopValue = 0;
                Assert::IsTrue(
                    Stack.Pop(&PopValue),
                    L"pop: pop failed");

                Assert::AreEqual<TPop>(
                    PopValue,
                    PopValueExpected,
                    L"push-pop result mismatch");
            }
            else if (PushTypeSize >= PopTypeSize)
            {
                TPop PopValue = 0;
                Assert::IsTrue(
                    Stack.PeekFrom(&PopValue, 0),
                    L"pop: peek failed");
                Assert::AreEqual<TPop>(
                    PopValue,
                    PopValueExpected,
                    L"push-pop result mismatch");

                Assert::IsTrue(
                    Stack.Pop<TPush>(nullptr),
                    L"pop: pop failed");
            }
            else if (PushTypeSize < PopTypeSize)
            {
                // convert TPush to TPop
                TPush PopValue = 0;
                Assert::IsTrue(
                    Stack.PeekFrom(&PopValue, 0),
                    L"pop: peek failed");
                Assert::AreEqual<TPop>(
                    static_cast<TPop>(PopValue),
                    PopValueExpected,
                    L"push-pop result mismatch");

                Assert::IsTrue(
                    Stack.Pop<TPush>(nullptr),
                    L"pop: pop failed");
            }
            else
            {
                Assert::Fail(L"unexpected test param");
            }

            Assert::AreEqual(
                OffsetBeforePush,
                Stack.TopOffset(),
                L"pop: unexpected stack top offset");

        }

        void Stack_Testcases(VMStack& Stack)
        {
            //
            // test push-pop for following primitive types:
            //   -> i1, i2, i4, i8, u1, u2, u4, u8
            //

            //
            // test push i1/i2/i4/i8 -> pop i1/i2/i4/i8/u1/u2/u4/u8
            //

            if (1)
            {
                // i1 -> i1/i2/i4/i8/u1/u2/u4/u8
                using TPushTest = int8_t;
                TPushTest v[] = { 0x12, 0x23, 0x34, 0x45 };

                Stack_TestHelper_PushPop<TPushTest, int8_t>(Stack, v[0], v[0] & 0xff);
                Stack_TestHelper_PushPop<TPushTest, int16_t>(Stack, v[1], v[1] & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, int32_t>(Stack, v[2], v[2] & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, int64_t>(Stack, v[3], v[3] & 0xffffffffffffffff);
                Stack_TestHelper_PushPop<TPushTest, int8_t>(Stack, Sign<TPushTest>(v[0]), Sign<TPushTest>(v[0]) & 0xff);
                Stack_TestHelper_PushPop<TPushTest, int16_t>(Stack, Sign<TPushTest>(v[1]), Sign<TPushTest>(v[1]) & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, int32_t>(Stack, Sign<TPushTest>(v[2]), Sign<TPushTest>(v[2]) & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, int64_t>(Stack, Sign<TPushTest>(v[3]), Sign<TPushTest>(v[3]) & 0xffffffffffffffff);

                Stack_TestHelper_PushPop<TPushTest, uint8_t>(Stack, v[0], v[0] & 0xff);
                Stack_TestHelper_PushPop<TPushTest, uint16_t>(Stack, v[1], v[1] & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, uint32_t>(Stack, v[2], v[2] & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, uint64_t>(Stack, v[3], v[3] & 0xffffffffffffffff);
                Stack_TestHelper_PushPop<TPushTest, uint8_t>(Stack, Sign<TPushTest>(v[0]), Sign<TPushTest>(v[0]) & 0xff);
                Stack_TestHelper_PushPop<TPushTest, uint16_t>(Stack, Sign<TPushTest>(v[1]), Sign<TPushTest>(v[1]) & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, uint32_t>(Stack, Sign<TPushTest>(v[2]), Sign<TPushTest>(v[2]) & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, uint64_t>(Stack, Sign<TPushTest>(v[3]), Sign<TPushTest>(v[3]) & 0xffffffffffffffff);
            }

            if (1)
            {
                // i2 -> i1/i2/i4/i8/u1/u2/u4/u8
                using TPushTest = int16_t;
                TPushTest v[] = { 0x1234, 0x2345, 0x3456, 0x4567 };

                Stack_TestHelper_PushPop<TPushTest, int8_t>(Stack, v[0], v[0] & 0xff);
                Stack_TestHelper_PushPop<TPushTest, int16_t>(Stack, v[1], v[1] & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, int32_t>(Stack, v[2], v[2] & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, int64_t>(Stack, v[3], v[3] & 0xffffffffffffffff);
                Stack_TestHelper_PushPop<TPushTest, int8_t>(Stack, Sign<TPushTest>(v[0]), Sign<TPushTest>(v[0]) & 0xff);
                Stack_TestHelper_PushPop<TPushTest, int16_t>(Stack, Sign<TPushTest>(v[1]), Sign<TPushTest>(v[1]) & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, int32_t>(Stack, Sign<TPushTest>(v[2]), Sign<TPushTest>(v[2]) & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, int64_t>(Stack, Sign<TPushTest>(v[3]), Sign<TPushTest>(v[3]) & 0xffffffffffffffff);

                Stack_TestHelper_PushPop<TPushTest, uint8_t>(Stack, v[0], v[0] & 0xff);
                Stack_TestHelper_PushPop<TPushTest, uint16_t>(Stack, v[1], v[1] & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, uint32_t>(Stack, v[2], v[2] & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, uint64_t>(Stack, v[3], v[3] & 0xffffffffffffffff);
                Stack_TestHelper_PushPop<TPushTest, uint8_t>(Stack, Sign<TPushTest>(v[0]), Sign<TPushTest>(v[0]) & 0xff);
                Stack_TestHelper_PushPop<TPushTest, uint16_t>(Stack, Sign<TPushTest>(v[1]), Sign<TPushTest>(v[1]) & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, uint32_t>(Stack, Sign<TPushTest>(v[2]), Sign<TPushTest>(v[2]) & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, uint64_t>(Stack, Sign<TPushTest>(v[3]), Sign<TPushTest>(v[3]) & 0xffffffffffffffff);
            }

            if (1)
            {
                // i4 -> i1/i2/i4/i8/u1/u2/u4/u8
                using TPushTest = int32_t;
                TPushTest v[] = { 0x12345678, 0x23456789, 0x3456789a, 0x456789ab };

                Stack_TestHelper_PushPop<TPushTest, int8_t>(Stack, v[0], v[0] & 0xff);
                Stack_TestHelper_PushPop<TPushTest, int16_t>(Stack, v[1], v[1] & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, int32_t>(Stack, v[2], v[2] & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, int64_t>(Stack, v[3], v[3] & 0xffffffffffffffff);
                Stack_TestHelper_PushPop<TPushTest, int8_t>(Stack, Sign<TPushTest>(v[0]), Sign<TPushTest>(v[0]) & 0xff);
                Stack_TestHelper_PushPop<TPushTest, int16_t>(Stack, Sign<TPushTest>(v[1]), Sign<TPushTest>(v[1]) & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, int32_t>(Stack, Sign<TPushTest>(v[2]), Sign<TPushTest>(v[2]) & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, int64_t>(Stack, Sign<TPushTest>(v[3]), Sign<TPushTest>(v[3]) & 0xffffffffffffffff);

                Stack_TestHelper_PushPop<TPushTest, uint8_t>(Stack, v[0], v[0] & 0xff);
                Stack_TestHelper_PushPop<TPushTest, uint16_t>(Stack, v[1], v[1] & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, uint32_t>(Stack, v[2], v[2] & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, uint64_t>(Stack, v[3], v[3] & 0xffffffffffffffff);
                Stack_TestHelper_PushPop<TPushTest, uint8_t>(Stack, Sign<TPushTest>(v[0]), Sign<TPushTest>(v[0]) & 0xff);
                Stack_TestHelper_PushPop<TPushTest, uint16_t>(Stack, Sign<TPushTest>(v[1]), Sign<TPushTest>(v[1]) & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, uint32_t>(Stack, Sign<TPushTest>(v[2]), Sign<TPushTest>(v[2]) & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, uint64_t>(Stack, Sign<TPushTest>(v[3]), Sign<TPushTest>(v[3]) & 0xffffffffffffffff);
            }

            if (1)
            {
                // i8 -> i1/i2/i4/i8/u1/u2/u4/u8
                using TPushTest = int64_t;
                TPushTest v[] = {
                    0x12345678'9abcdef0, 0x23456789'abcdef01,
                    0x3456789a'bcdef012, 0x456789ab'cdef0123
                };

                Stack_TestHelper_PushPop<TPushTest, int8_t>(Stack, v[0], v[0] & 0xff);
                Stack_TestHelper_PushPop<TPushTest, int16_t>(Stack, v[1], v[1] & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, int32_t>(Stack, v[2], v[2] & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, int64_t>(Stack, v[3], v[3] & 0xffffffffffffffff);
                Stack_TestHelper_PushPop<TPushTest, int8_t>(Stack, Sign<TPushTest>(v[0]), Sign<TPushTest>(v[0]) & 0xff);
                Stack_TestHelper_PushPop<TPushTest, int16_t>(Stack, Sign<TPushTest>(v[1]), Sign<TPushTest>(v[1]) & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, int32_t>(Stack, Sign<TPushTest>(v[2]), Sign<TPushTest>(v[2]) & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, int64_t>(Stack, Sign<TPushTest>(v[3]), Sign<TPushTest>(v[3]) & 0xffffffffffffffff);

                Stack_TestHelper_PushPop<TPushTest, uint8_t>(Stack, v[0], v[0] & 0xff);
                Stack_TestHelper_PushPop<TPushTest, uint16_t>(Stack, v[1], v[1] & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, uint32_t>(Stack, v[2], v[2] & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, uint64_t>(Stack, v[3], v[3] & 0xffffffffffffffff);
                Stack_TestHelper_PushPop<TPushTest, uint8_t>(Stack, Sign<TPushTest>(v[0]), Sign<TPushTest>(v[0]) & 0xff);
                Stack_TestHelper_PushPop<TPushTest, uint16_t>(Stack, Sign<TPushTest>(v[1]), Sign<TPushTest>(v[1]) & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, uint32_t>(Stack, Sign<TPushTest>(v[2]), Sign<TPushTest>(v[2]) & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, uint64_t>(Stack, Sign<TPushTest>(v[3]), Sign<TPushTest>(v[3]) & 0xffffffffffffffff);
            }


            //
            // test push u1/u2/u4/u8 -> pop i1/i2/i4/i8/u1/u2/u4/u8
            //

            if (1)
            {
                // u1 -> i1/i2/i4/i8/u1/u2/u4/u8
                using TPushTest = uint8_t;
                TPushTest v[] = { 0x12, 0x23, 0x34, 0x45 };

                Stack_TestHelper_PushPop<TPushTest, int8_t>(Stack, v[0], v[0] & 0xff);
                Stack_TestHelper_PushPop<TPushTest, int16_t>(Stack, v[1], v[1] & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, int32_t>(Stack, v[2], v[2] & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, int64_t>(Stack, v[3], v[3] & 0xffffffffffffffff);
                Stack_TestHelper_PushPop<TPushTest, int8_t>(Stack, Sign<TPushTest>(v[0]), Sign<TPushTest>(v[0]) & 0xff);
                Stack_TestHelper_PushPop<TPushTest, int16_t>(Stack, Sign<TPushTest>(v[1]), Sign<TPushTest>(v[1]) & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, int32_t>(Stack, Sign<TPushTest>(v[2]), Sign<TPushTest>(v[2]) & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, int64_t>(Stack, Sign<TPushTest>(v[3]), Sign<TPushTest>(v[3]) & 0xffffffffffffffff);

                Stack_TestHelper_PushPop<TPushTest, uint8_t>(Stack, v[0], v[0] & 0xff);
                Stack_TestHelper_PushPop<TPushTest, uint16_t>(Stack, v[1], v[1] & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, uint32_t>(Stack, v[2], v[2] & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, uint64_t>(Stack, v[3], v[3] & 0xffffffffffffffff);
                Stack_TestHelper_PushPop<TPushTest, uint8_t>(Stack, Sign<TPushTest>(v[0]), Sign<TPushTest>(v[0]) & 0xff);
                Stack_TestHelper_PushPop<TPushTest, uint16_t>(Stack, Sign<TPushTest>(v[1]), Sign<TPushTest>(v[1]) & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, uint32_t>(Stack, Sign<TPushTest>(v[2]), Sign<TPushTest>(v[2]) & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, uint64_t>(Stack, Sign<TPushTest>(v[3]), Sign<TPushTest>(v[3]) & 0xffffffffffffffff);
            }

            if (1)
            {
                // u2 -> i1/i2/i4/i8/u1/u2/u4/u8
                using TPushTest = uint16_t;
                TPushTest v[] = { 0x1234, 0x2345, 0x3456, 0x4567 };

                Stack_TestHelper_PushPop<TPushTest, int8_t>(Stack, v[0], v[0] & 0xff);
                Stack_TestHelper_PushPop<TPushTest, int16_t>(Stack, v[1], v[1] & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, int32_t>(Stack, v[2], v[2] & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, int64_t>(Stack, v[3], v[3] & 0xffffffffffffffff);
                Stack_TestHelper_PushPop<TPushTest, int8_t>(Stack, Sign<TPushTest>(v[0]), Sign<TPushTest>(v[0]) & 0xff);
                Stack_TestHelper_PushPop<TPushTest, int16_t>(Stack, Sign<TPushTest>(v[1]), Sign<TPushTest>(v[1]) & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, int32_t>(Stack, Sign<TPushTest>(v[2]), Sign<TPushTest>(v[2]) & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, int64_t>(Stack, Sign<TPushTest>(v[3]), Sign<TPushTest>(v[3]) & 0xffffffffffffffff);

                Stack_TestHelper_PushPop<TPushTest, uint8_t>(Stack, v[0], v[0] & 0xff);
                Stack_TestHelper_PushPop<TPushTest, uint16_t>(Stack, v[1], v[1] & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, uint32_t>(Stack, v[2], v[2] & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, uint64_t>(Stack, v[3], v[3] & 0xffffffffffffffff);
                Stack_TestHelper_PushPop<TPushTest, uint8_t>(Stack, Sign<TPushTest>(v[0]), Sign<TPushTest>(v[0]) & 0xff);
                Stack_TestHelper_PushPop<TPushTest, uint16_t>(Stack, Sign<TPushTest>(v[1]), Sign<TPushTest>(v[1]) & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, uint32_t>(Stack, Sign<TPushTest>(v[2]), Sign<TPushTest>(v[2]) & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, uint64_t>(Stack, Sign<TPushTest>(v[3]), Sign<TPushTest>(v[3]) & 0xffffffffffffffff);
            }

            if (1)
            {
                // u4 -> i1/i2/i4/i8/u1/u2/u4/u8
                using TPushTest = uint32_t;
                TPushTest v[] = { 0x12345678, 0x23456789, 0x3456789a, 0x456789ab };

                Stack_TestHelper_PushPop<TPushTest, int8_t>(Stack, v[0], v[0] & 0xff);
                Stack_TestHelper_PushPop<TPushTest, int16_t>(Stack, v[1], v[1] & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, int32_t>(Stack, v[2], v[2] & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, int64_t>(Stack, v[3], v[3] & 0xffffffffffffffff);
                Stack_TestHelper_PushPop<TPushTest, int8_t>(Stack, Sign<TPushTest>(v[0]), Sign<TPushTest>(v[0]) & 0xff);
                Stack_TestHelper_PushPop<TPushTest, int16_t>(Stack, Sign<TPushTest>(v[1]), Sign<TPushTest>(v[1]) & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, int32_t>(Stack, Sign<TPushTest>(v[2]), Sign<TPushTest>(v[2]) & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, int64_t>(Stack, Sign<TPushTest>(v[3]), Sign<TPushTest>(v[3]) & 0xffffffffffffffff);

                Stack_TestHelper_PushPop<TPushTest, uint8_t>(Stack, v[0], v[0] & 0xff);
                Stack_TestHelper_PushPop<TPushTest, uint16_t>(Stack, v[1], v[1] & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, uint32_t>(Stack, v[2], v[2] & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, uint64_t>(Stack, v[3], v[3] & 0xffffffffffffffff);
                Stack_TestHelper_PushPop<TPushTest, uint8_t>(Stack, Sign<TPushTest>(v[0]), Sign<TPushTest>(v[0]) & 0xff);
                Stack_TestHelper_PushPop<TPushTest, uint16_t>(Stack, Sign<TPushTest>(v[1]), Sign<TPushTest>(v[1]) & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, uint32_t>(Stack, Sign<TPushTest>(v[2]), Sign<TPushTest>(v[2]) & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, uint64_t>(Stack, Sign<TPushTest>(v[3]), Sign<TPushTest>(v[3]) & 0xffffffffffffffff);
            }

            if (1)
            {
                // u8 -> i1/i2/i4/i8/u1/u2/u4/u8
                using TPushTest = uint64_t;
                TPushTest v[] = {
                    0x12345678'9abcdef0, 0x23456789'abcdef01,
                    0x3456789a'bcdef012, 0x456789ab'cdef0123
                };

                Stack_TestHelper_PushPop<TPushTest, int8_t>(Stack, v[0], v[0] & 0xff);
                Stack_TestHelper_PushPop<TPushTest, int16_t>(Stack, v[1], v[1] & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, int32_t>(Stack, v[2], v[2] & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, int64_t>(Stack, v[3], v[3] & 0xffffffffffffffff);
                Stack_TestHelper_PushPop<TPushTest, int8_t>(Stack, Sign<TPushTest>(v[0]), Sign<TPushTest>(v[0]) & 0xff);
                Stack_TestHelper_PushPop<TPushTest, int16_t>(Stack, Sign<TPushTest>(v[1]), Sign<TPushTest>(v[1]) & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, int32_t>(Stack, Sign<TPushTest>(v[2]), Sign<TPushTest>(v[2]) & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, int64_t>(Stack, Sign<TPushTest>(v[3]), Sign<TPushTest>(v[3]) & 0xffffffffffffffff);

                Stack_TestHelper_PushPop<TPushTest, uint8_t>(Stack, v[0], v[0] & 0xff);
                Stack_TestHelper_PushPop<TPushTest, uint16_t>(Stack, v[1], v[1] & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, uint32_t>(Stack, v[2], v[2] & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, uint64_t>(Stack, v[3], v[3] & 0xffffffffffffffff);
                Stack_TestHelper_PushPop<TPushTest, uint8_t>(Stack, Sign<TPushTest>(v[0]), Sign<TPushTest>(v[0]) & 0xff);
                Stack_TestHelper_PushPop<TPushTest, uint16_t>(Stack, Sign<TPushTest>(v[1]), Sign<TPushTest>(v[1]) & 0xffff);
                Stack_TestHelper_PushPop<TPushTest, uint32_t>(Stack, Sign<TPushTest>(v[2]), Sign<TPushTest>(v[2]) & 0xffffffff);
                Stack_TestHelper_PushPop<TPushTest, uint64_t>(Stack, Sign<TPushTest>(v[3]), Sign<TPushTest>(v[3]) & 0xffffffffffffffff);
            }
        }
    };

    TEST_CLASS(VMMemoryTest)
    {
    public:
        TEST_METHOD(Memory_MasterTest)
        {
            enum TestType
            {
                TestNull,
                TestLog,
                TestAlloc,
                TestFree,
            };

            struct AllocParams
            {
                AllocParams() :
                    PreferredAddress(), Size(), UsePreferredAddress(),
                    ExpectedSuccess(), ResultAddress() {}

                AllocParams(uint64_t PreferredAddress, size_t Size, bool UsePreferredAddress, bool ExpectedSuccess) :
                    PreferredAddress(PreferredAddress), Size(Size), UsePreferredAddress(UsePreferredAddress),
                    ExpectedSuccess(ExpectedSuccess), ResultAddress() {}

                uint64_t PreferredAddress;
                size_t Size;
                bool UsePreferredAddress;
                bool ExpectedSuccess;
                uint64_t ResultAddress;
            };

            struct FreeParams
            {
                FreeParams() :
                    Address(), Size(), FreeAll(), ExpectedSuccess() {}

                FreeParams(uint64_t Address, size_t Size, bool PartialFree, bool FreeAll, bool ExpectedSuccess) :
                    Address(Address), Size(Size), PartialFree(PartialFree), FreeAll(FreeAll), ExpectedSuccess(ExpectedSuccess) {}

                uint64_t Address;
                size_t Size;
                bool PartialFree;
                bool FreeAll;
                bool ExpectedSuccess;
            };

            struct LogParams
            {
                LogParams() : Text{} {}
                LogParams(const char* p)
                {
                    strcpy_s(Text, p);
                }

                LogParams(const std::string& s) : LogParams(s.c_str()) { }

                char Text[128];
            };

            struct AllocationTest
            {
                AllocationTest() :
                    Test(TestNull), Alloc{}, Free{}, Log{} {}

                AllocationTest(const AllocParams& Params) :
                    Test(TestAlloc), Alloc(Params), Free{}, Log{} {}

                AllocationTest(const FreeParams& Params) :
                    Test(TestFree), Alloc{}, Free(Params), Log{} {}

                AllocationTest(const LogParams& Params) :
                    Test(TestLog), Alloc{}, Free{}, Log(Params) {}

                TestType Test;
                AllocParams Alloc;
                FreeParams Free;
                LogParams Log;
            };

            size_t MemoryTotalSize = 0x000a0000;


            std::vector<AllocationTest> TestInput
            {
                //
                // 1-1. 0x00000000 - 0x0009ffff full allocation (address unspecified)
                //

                // full allocation test
                LogParams("0x00000000 - 0x0009ffff full allocation (address unspecified)"),
                AllocParams(0x00000000, 0x00020000, false, true),
                AllocParams(0x00000000, 0x00020000, false, true), 
                AllocParams(0x00000000, 0x00020000, false, true), 
                AllocParams(0x00000000, 0x00020000, false, true), 
                AllocParams(0x00000000, 0x00020000, false, true), 

                // fail test 1
                LogParams("fail test 1"),
                AllocParams(0x00040000 - 0x00010000, 0x00010000, true, false), // fail!
                AllocParams(0x00041000 - 0x00010000, 0x0000f000, true, false), // fail!
                AllocParams(0x00042000 - 0x00010000, 0x0000e000, true, false), // fail!
                AllocParams(0x0004e000 - 0x00010000, 0x00002000, true, false), // fail!
                AllocParams(0x0004f000 - 0x00010000, 0x00001000, true, false), // fail!

                AllocParams(0x00040000 - 0x00008000, 0x00010000, true, false), // fail!
                AllocParams(0x00041000 - 0x00008000, 0x0000f000, true, false), // fail!
                AllocParams(0x00042000 - 0x00008000, 0x0000e000, true, false), // fail!
                AllocParams(0x0004e000 - 0x00008000, 0x00002000, true, false), // fail!
                AllocParams(0x0004f000 - 0x00008000, 0x00001000, true, false), // fail!

                AllocParams(0x00040000 + 0x00000000, 0x00010000, true, false), // fail!
                AllocParams(0x00041000 + 0x00000000, 0x0000f000, true, false), // fail!
                AllocParams(0x00042000 + 0x00000000, 0x0000e000, true, false), // fail!
                AllocParams(0x0004e000 + 0x00000000, 0x00002000, true, false), // fail!
                AllocParams(0x0004f000 + 0x00000000, 0x00001000, true, false), // fail!

                AllocParams(0x00040000 + 0x00008000, 0x00010000, true, false), // fail!
                AllocParams(0x00041000 + 0x00008000, 0x0000f000, true, false), // fail!
                AllocParams(0x00042000 + 0x00008000, 0x0000e000, true, false), // fail!
                AllocParams(0x0004e000 + 0x00008000, 0x00002000, true, false), // fail!
                AllocParams(0x0004f000 + 0x00008000, 0x00001000, true, false), // fail!

                AllocParams(0x00040000 + 0x00010000, 0x00010000, true, false), // fail!
                AllocParams(0x00041000 + 0x00010000, 0x0000f000, true, false), // fail!
                AllocParams(0x00042000 + 0x00010000, 0x0000e000, true, false), // fail!
                AllocParams(0x0004e000 + 0x00010000, 0x00002000, true, false), // fail!
                AllocParams(0x0004f000 + 0x00010000, 0x00001000, true, false), // fail!

                // fail test 2
                LogParams("fail test 2"),
                AllocParams(0x00040000 - 0x00010000, 0x00080000, true, false), // fail!
                AllocParams(0x00040000 - 0x00010000, 0x00040000, true, false), // fail!
                AllocParams(0x00040000 - 0x00010000, 0x00020000, true, false), // fail!

                AllocParams(0x00040000 - 0x00008000, 0x00080000, true, false), // fail!
                AllocParams(0x00040000 - 0x00008000, 0x00040000, true, false), // fail!
                AllocParams(0x00040000 - 0x00008000, 0x00020000, true, false), // fail!

                AllocParams(0x00040000 + 0x00000000, 0x00080000, true, false), // fail!
                AllocParams(0x00040000 + 0x00000000, 0x00040000, true, false), // fail!
                AllocParams(0x00040000 + 0x00000000, 0x00020000, true, false), // fail!

                AllocParams(0x00040000 + 0x00008000, 0x00080000, true, false), // fail!
                AllocParams(0x00040000 + 0x00008000, 0x00040000, true, false), // fail!
                AllocParams(0x00040000 + 0x00008000, 0x00020000, true, false), // fail!

                AllocParams(0x00040000 + 0x00010000, 0x00080000, true, false), // fail!
                AllocParams(0x00040000 + 0x00010000, 0x00040000, true, false), // fail!
                AllocParams(0x00040000 + 0x00010000, 0x00020000, true, false), // fail!

                //
                // 1-2. 0x00000000 - 0x0009ffff free all
                //

                LogParams("0x00000000 - 0x0009ffff free all"),
                FreeParams(0x00000000, 0x00020000, false, false, true),
                FreeParams(0x00020000, 0x00020000, false, false, true),
                FreeParams(0x00040000, 0x00020000, false, false, true),
                FreeParams(0x00060000, 0x00020000, false, false, true),
                FreeParams(0x00080000, 0x00020000, false, false, true),


                //
                // 2-1. 0x00000000 - 0x0009ffff full allocation (address specified)
                //

                LogParams("0x00000000 - 0x0009ffff full allocation (address specified)"),
                AllocParams(0x00000000, 0x00020000, true, true),
                AllocParams(0x00020000, 0x00020000, true, true), 
                AllocParams(0x00040000, 0x00020000, true, true), 
                AllocParams(0x00060000, 0x00020000, true, true), 
                AllocParams(0x00080000, 0x00020000, true, true), 

                //
                // 2-2. 0x00000000 - 0x0009ffff free all
                //

                LogParams("0x00000000 - 0x0009ffff free all"),
                FreeParams(0x00000000, 0x00020000, false, false, true),
                FreeParams(0x00020000, 0x00020000, false, false, true),
                FreeParams(0x00040000, 0x00020000, false, false, true),
                FreeParams(0x00060000, 0x00020000, false, false, true),
                FreeParams(0x00080000, 0x00020000, false, false, true),


                //
                // 3-1. 0x00000000 - 0x0009ffff full allocation (address specified/unspecified mix)
                //

                LogParams("0x00000000 - 0x0009ffff full allocation (address specified/unspecified mix)"),
                AllocParams(0x00010000, 0x00010000, true, true),
                AllocParams(0x00030000, 0x00010000, true, true),
                AllocParams(0x00050000, 0x00010000, true, true),
                AllocParams(0x00070000, 0x00010000, true, true),
                AllocParams(0x00090000, 0x00010000, true, true),

                AllocParams(0x00000000, 0x00011000, false, false), // fail!
                AllocParams(0x00000000, 0x00012000, false, false), // fail!
                AllocParams(0x00000000, 0x00020000, false, false), // fail!

                AllocParams(0x00020000, 0x00011000, true, false), // fail!
                AllocParams(0x00020000, 0x00012000, true, false), // fail!
                AllocParams(0x00020000, 0x00020000, true, false), // fail!
                AllocParams(0x00030000, 0x00011000, true, false), // fail!
                AllocParams(0x00030000, 0x00012000, true, false), // fail!
                AllocParams(0x00030000, 0x00020000, true, false), // fail!

                AllocParams(0x00000000, 0x00010000, false, true), // one of (00000000, 00020000, 00040000, 00060000, 00080000)
                AllocParams(0x00000000, 0x00010000, false, true), // one of (00000000, 00020000, 00040000, 00060000, 00080000)
                AllocParams(0x00000000, 0x00010000, false, true), // one of (00000000, 00020000, 00040000, 00060000, 00080000)
                AllocParams(0x00000000, 0x00010000, false, true), // one of (00000000, 00020000, 00040000, 00060000, 00080000)
                AllocParams(0x00000000, 0x00010000, false, true), // one of (00000000, 00020000, 00040000, 00060000, 00080000)

                //
                // 3-2. 0x00000000 - 0x0009ffff free all
                //

                LogParams("0x00000000 - 0x0009ffff free all"),
                FreeParams(0x00000000, 0x00010000, false, false, true),
                FreeParams(0x00010000, 0x00010000, false, false, true),
                FreeParams(0x00020000, 0x00010000, false, false, true),
                FreeParams(0x00030000, 0x00010000, false, false, true),
                FreeParams(0x00040000, 0x00010000, false, false, true),
                FreeParams(0x00050000, 0x00010000, false, false, true),
                FreeParams(0x00060000, 0x00010000, false, false, true),
                FreeParams(0x00070000, 0x00010000, false, false, true),
                FreeParams(0x00080000, 0x00010000, false, false, true),
                FreeParams(0x00090000, 0x00010000, false, false, true),


                //
                // 4-1. 0x00000000 - 0x0009ffff full allocation (different block size)
                //
                
                LogParams("0x00000000 - 0x0009ffff full allocation (different block size)"),
                AllocParams(0x00000000, 0x00100000, false, false), // fail!
                AllocParams(0x00000000, 0x00040000, false, true),
                AllocParams(0x00000000, 0x00020000, false, true),
                AllocParams(0x00000000, 0x00020000, false, true),
                AllocParams(0x00000000, 0x00010000, false, true),
                AllocParams(0x00000000, 0x00008000, false, true),
                AllocParams(0x00000000, 0x00004000, false, true),
                AllocParams(0x00000000, 0x00002000, false, true),
                AllocParams(0x00000000, 0x00001000, false, true),
                AllocParams(0x00000000, 0x00001000, false, true),
                AllocParams(0x00000000, 0x00001000, false, false), // fail!

                //
                // 4-2. 0x00000000 - 0x0009ffff free all
                //

                LogParams("0x00000000 - 0x0009ffff free all"),
                FreeParams(0, 0, false, true, true),


                //
                // 5-1. 0x00000000 - 0x0009ffff full allocation + partial free
                //
                
                LogParams("0x00000000 - 0x0009ffff full allocation + partial free"),

                // 0123456789
                // oooooooooo -> 0..9
                AllocParams(0x00000000, 0x000a0000, false, true),

                // 0123456789
                // oxooooooxo -> 0, 2..7, 9
                FreeParams(0x00010000, 0x00010000, true, false, true),
                FreeParams(0x00080000, 0x00010000, true, false, true),

                // 0123456789
                // oxoxxxxoxo -> 0, 2, 7, 9
                FreeParams(0x00030000, 0x00040000, true, false, true),
                FreeParams(0x00000000, 0x000a0000, true, false, false), // fail!

                // 0123456789
                // xxxxxxxxxx -> (empty)
                FreeParams(0x00000000, 0x00010000, true, false, true),
                FreeParams(0x00020000, 0x00010000, true, false, true),
                FreeParams(0x00070000, 0x00010000, true, false, true),
                FreeParams(0x00090000, 0x00010000, true, false, true),

                // 0123456789
                // oooooooooo -> 0..9
                AllocParams(0x00000000, 0x000a0000, false, true),

                //
                // 5-2. 0x00000000 - 0x0009ffff free all
                //

                LogParams("0x00000000 - 0x0009ffff free all"),
                FreeParams(0x00000000, 0x000a0000, false, false, true),


                //
                // 6-1. double-free test
                //
                    
                LogParams("double-free test"),
                FreeParams(0x00000000, 0x000a0000, false, false, false),

            };

            struct BlockInfo
            {
                BlockInfo(uint64_t Base, uint64_t Size) :
                    Base(Base), Size(Size) {}
                uint64_t Base;
                uint64_t Size;
            };

            VMMemoryManager Memory(MemoryTotalSize);

            std::vector<BlockInfo> AllocatedBlocks;

            for (int Index = 0; Index < TestInput.size(); Index++)
            {
                auto& Input = TestInput[Index];
                bool BlockListValidationNeeded = false;

                switch (Input.Test)
                {
                case TestNull:
                {
                    break;
                }
                case TestAlloc:
                {
                    Logger::WriteMessage(Format("TestAlloc for test input index %d", Index).c_str());

                    bool Succeeded = Memory.Allocate(
                        Input.Alloc.PreferredAddress, Input.Alloc.Size, MemoryType::Data, 0,
                        Input.Alloc.UsePreferredAddress ? VMMemoryManager::Options::UsePreferredAddress : 0,
                        Input.Alloc.ResultAddress);

                    Assert::AreEqual(
                        Succeeded, Input.Alloc.ExpectedSuccess,
                        L"memory allocation result mismatch"
                    );

                    if (Succeeded)
                    {
                        if (Input.Alloc.UsePreferredAddress)
                        {
                            Assert::AreEqual(
                                Input.Alloc.PreferredAddress, Input.Alloc.ResultAddress,
                                L"allocated memory address mismatch"
                            );
                        }

                        AllocatedBlocks.push_back(BlockInfo(Input.Alloc.ResultAddress, Input.Alloc.Size));
                        BlockListValidationNeeded = true;
                    }
                    break;
                }
                case TestFree:
                {
                    Logger::WriteMessage(Format("TestFree for test input index %d", Index).c_str());

                    if (Input.Free.FreeAll)
                    {
                        // NOTE: not working if partial-freed block exists
                        for (int i = 0; i < AllocatedBlocks.size(); i++)
                        {
                            auto& it = AllocatedBlocks[i];

                            uint64_t FreedSize = Memory.Free(it.Base, it.Size);
                            uint64_t ExpectedFreedSize = Input.Free.ExpectedSuccess ? it.Size : 0;
                            Assert::IsTrue(
                                FreedSize == ExpectedFreedSize,
                                L"freed size result mismatch"
                            );

                            it.Size = 0; // mark block is freed

                            if (!Input.Free.PartialFree)
                            {
                                // do block validation
                                for (int j = i + 1; j < AllocatedBlocks.size(); j++)
                                {
                                    auto& it2 = AllocatedBlocks[j];

                                    MemoryInfo Info{};
                                    Assert::IsTrue(
                                        Memory.Query(it2.Base, Info),
                                        L"block validation failure"
                                    );
                                    Assert::IsTrue(
                                        it2.Base == Info.Base,
                                        L"block validation failure (base address mismatch; maybe block is partial-freed)"
                                    );
                                    Assert::IsTrue(
                                        it2.Size == Info.Size,
                                        L"block validation failure (block size mismatch; maybe block is partial-freed)"
                                    );
                                }
                            }
                        }

                        AllocatedBlocks.clear();
                    }
                    else
                    {
                        uint64_t FreedSize = Memory.Free(Input.Free.Address, Input.Free.Size);
                        uint64_t ExpectedFreedSize = Input.Free.ExpectedSuccess ? Input.Free.Size : 0;
                        Assert::IsTrue(
                            FreedSize == ExpectedFreedSize,
                            L"freed size result mismatch"
                        );

                        if (!Input.Free.PartialFree)
                        {
                            if (Input.Free.ExpectedSuccess)
                            {
                                AllocatedBlocks.erase(
                                    std::remove_if(AllocatedBlocks.begin(), AllocatedBlocks.end(),
                                        [Target = Input.Free.Address](auto it) { return Target == it.Base; }),
                                    AllocatedBlocks.end()
                                );
                            }

                            BlockListValidationNeeded = true;
                        }
                    }
                    break;
                }
                case TestLog:
                {
                    Logger::WriteMessage(Input.Log.Text);
                    break;
                }
                }

                if (BlockListValidationNeeded)
                {
                    // do block validation
                    for (auto& it : AllocatedBlocks)
                    {
                        MemoryInfo Info{};
                        Assert::IsTrue(
                            Memory.Query(it.Base, Info),
                            L"block validation failure"
                        );
                        Assert::IsTrue(
                            it.Base == Info.Base,
                            L"block validation failure (base address mismatch; maybe block is partial-freed)"
                        );
                        Assert::IsTrue(
                            it.Size == Info.Size,
                            L"block validation failure (block size mismatch; maybe block is partial-freed)"
                        );
                    }
                }
            }
        }

    private:
    };

    TEST_CLASS(VMInterpreterTest)
    {
    public:
        struct GuestMemory
        {
            GuestMemory() :
                Address(), Size() {}
            GuestMemory(uint64_t Address, uint64_t Size) :
                Address(Address), Size(Size) {}

            uint64_t Address;
            uint64_t Size;
        };

        struct EmitInfo
        {
            EmitInfo(bool ExpectTrap, Opcode::T Code) :
                ExpectTrap(ExpectTrap), Code(Code), Op() {}
            EmitInfo(bool ExpectTrap, Opcode::T Code, Operand Op) :
                ExpectTrap(ExpectTrap), Code(Code), Op(Op) {}

            bool ExpectTrap; // expects instruction may cause trap (including bp instruction)
            Opcode::T Code;
            Operand Op;
        };

        struct StackState
        {
            StackState(bool Invalid = false) :
                Invalid(Invalid), PushedCount(), Pushed{} {}
            StackState(int32_t PoppedCount) :
                Invalid(false), PushedCount(-PoppedCount), Pushed{} {}
            StackState(int32_t PushedCount, std::vector<uint64_t> Pushed) :
                Invalid(false), PushedCount(PushedCount), Pushed(Pushed) {}
            StackState(std::vector<uint64_t> Pushed) :
                Invalid(false), PushedCount(static_cast<int32_t>(Pushed.size())), Pushed(Pushed) {}

            static StackState InvalidStackState()
            {
                return StackState(true);
            }

            bool Invalid;
            int32_t PushedCount;
            std::vector<uint64_t> Pushed;
        };

        static void OpenConsole()
        {
            for (int i = 0; i < 5; i++)
            {
                if (::AllocConsole())
                    break;

                ::FreeConsole();
            }

            HWND hWndConsole = ::GetConsoleWindow();

            ShowWindow(hWndConsole, SW_SHOW);
            ShowWindow(hWndConsole, SW_SHOW); // once again

            freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN", "r", stdin);
            freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
            freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr);
        }

        static void HideConsole()
        {
            HWND hWndConsole = ::GetConsoleWindow();
            ShowWindow(hWndConsole, SW_HIDE);
        }

        TEST_CLASS_INITIALIZE(ClassInitialize)
        {
            OpenConsole();
        }

        TEST_CLASS_CLEANUP(ClassCleanup)
        {
            printf("Test end.\nPress any key to continue\n");
            _getch();

            HideConsole();
        }

        TEST_METHOD_INITIALIZE(MethodInitialize)
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

            //
            // Allocate guest memory.
            //

            size_t MemorySize = 0x4000000;
            auto VMM = std::make_unique<VMMemoryManager>(MemorySize);
            auto& Memory = *VMM.get();

            for (auto& it : AllocParamTable)
            {
                if (!Memory.Allocate(it.PreferredAddress, it.Size, it.Type, it.Tag, it.Options, it.ResultAddress))
                {
                    Logger::WriteMessage(Format(
                        "failed to allocate guest memory for %s", it.Description).c_str());
                    return;
                }

                // touch
                Logger::WriteMessage(Format(
                    "touching guest memory 0x%016llx - 0x%016llx (%s)",
                    it.ResultAddress, it.ResultAddress + it.Size - 1, it.Description).c_str());
                Memory.Fill(it.ResultAddress, it.Size, 0xdd);
            }

            //
            // Setup initial VM context.
            //

            const int DefaultAlignment = sizeof(intptr_t); // Stack alignment

            VMExecutionContext ExecutionContext{};
            ExecutionContext.IP = static_cast<uint32_t>(GuestCode.ResultAddress);
            ExecutionContext.FetchedPrefix = 0;
            ExecutionContext.XTableState = 0;
            ExecutionContext.ExceptionState = ExceptionState::T::None;
            ExecutionContext.Mode = 0;

            ExecutionContext.Stack = VMStack(
                Memory.HostAddress(GuestStack.ResultAddress), GuestStack.Size, DefaultAlignment);
            ExecutionContext.ShadowStack = VMStack(
                Memory.HostAddress(GuestShadowStack.ResultAddress), GuestShadowStack.Size, DefaultAlignment);
            ExecutionContext.LocalVariableStack = VMStack(
                Memory.HostAddress(GuestLocalVarStack.ResultAddress), GuestLocalVarStack.Size, DefaultAlignment);
            ExecutionContext.ArgumentStack = VMStack(
                Memory.HostAddress(GuestArgumentStack.ResultAddress), GuestArgumentStack.Size, DefaultAlignment);

            if (DefaultAlignment == sizeof(int64_t))
            {
                ExecutionContext.Mode |= ModeBits::T::VMStackOper64Bit; // 64-bit stack oper
            }

            GuestCode_ = GuestMemory(GuestCode.ResultAddress, GuestCode.Size);
            GuestStack_ = GuestMemory(GuestStack.ResultAddress, GuestStack.Size);
            GuestShadowStack_ = GuestMemory(GuestShadowStack.ResultAddress, GuestShadowStack.Size);
            GuestLocalVarStack_ = GuestMemory(GuestLocalVarStack.ResultAddress, GuestLocalVarStack.Size);
            GuestArgumentStack_ = GuestMemory(GuestArgumentStack.ResultAddress, GuestArgumentStack.Size);

            ExecutionContext_ = ExecutionContext;
            ExecutionContextInitial_ = ExecutionContext;
            Memory_ = std::move(VMM);
        }

        TEST_METHOD_CLEANUP(MethodCleanup)
        {
            Memory_ = nullptr;
            ExecutionContext_ = {};
        }

        static_assert(Opcode::T::Bp == 1, "unexpected opcode value");

        template <
            typename T,
            std::enable_if_t<std::is_integral<T>::value && sizeof(T) == 1, bool> = true>
            Operand OperandHelper(T Value)
        {
            return Operand(OperandType::Imm8, Value);
        }

        template <
            typename T,
            std::enable_if_t<std::is_integral<T>::value && sizeof(T) == 2, bool> = true>
            Operand OperandHelper(T Value)
        {
            return Operand(OperandType::Imm16, Value);
        }

        template <
            typename T,
            std::enable_if_t<std::is_integral<T>::value && sizeof(T) == 4, bool> = true>
            Operand OperandHelper(T Value)
        {
            return Operand(OperandType::Imm32, Value);
        }

        template <
            typename T,
            std::enable_if_t<std::is_integral<T>::value && sizeof(T) == 8, bool> = true>
            Operand OperandHelper(T Value)
        {
            return Operand(OperandType::Imm64, Value);
        }

        template <
            typename T,
            std::enable_if_t<std::is_integral<T>::value, bool> = true>
            T ToBinaryRepr(T Value)
        {
            return Value;
        }

        template <
            typename T,
            std::enable_if_t<std::is_same<T, float>::value, bool> = true>
            uint32_t ToBinaryRepr(T Value)
        {
            return Base::BitCast<uint32_t>(Value);
        }

        template <
            typename T,
            std::enable_if_t<std::is_same<T, double>::value, bool> = true>
            uint64_t ToBinaryRepr(T Value)
        {
            return Base::BitCast<uint64_t>(Value);
        }


        template <typename T>
        void VerifyStack(VMStack& Stack, T Expected)
        {
            static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value, 
                "T is neither integral nor floating point");
            T Result = 0;
            Assert::IsTrue(Stack.Pop(&Result));
            Assert::AreEqual(Result, Expected, L"result mismatch");
        }

        template <typename T>
        void VerifyStackHelper(VMStack& Stack, T Expected)
        {
            VerifyStack(Stack, Expected);
        }

        template <
            typename T,
            typename... TRest, 
            typename = std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>,
            typename = std::enable_if_t<std::is_integral<TRest...>::value || std::is_floating_point<TRest...>::value>>
            void VerifyStackHelper(VMStack& Stack, T Expected, TRest... Rest)
        {
            VerifyStackHelper(Stack, Expected);

            return VerifyStackHelper(Stack, Rest...);
        }

        void DoSingleTest_New(
            const VMExecutionContext& InitialContext, VMExecutionContext& ResultContext,
            std::vector<EmitInfo> EmitOp, uint32_t ExpectedOutputResultSize, ExceptionState::T ExpectedExceptionState,
            StackState ExpectedStackState, StackState ExpectedArgumentStackState,
            StackState ExpectedLocalVarStackState)
        {
            VMExecutionContext Context = InitialContext;

            uint32_t PrevIP = Context.IP;
            uint32_t PrevStackTop = Context.Stack.TopOffset();
            uint32_t PrevArgumentStackTop = Context.ArgumentStack.TopOffset();
            uint32_t PrevLocalVarStackTop = Context.LocalVariableStack.TopOffset();

            uint32_t CodeSize = static_cast<uint32_t>(GuestCode_.Size);
            unsigned char* HostAddress = reinterpret_cast<unsigned char*>
                (Memory_->HostAddress(Context.IP, CodeSize));

            size_t ResultSize = 0;
            uint32_t TotalEmitSize = 0;
            uint32_t ExpectedIPOffset = 0;
            int ExpectedStepCount = 0;
            int TotalEmitCount = 0;
            bool ExpectedTrap = false;

            VMBytecodeEmitter Emitter;

            for (auto& it : EmitOp)
            {
                if (it.ExpectTrap)
                {
                    if (!ExpectedTrap)
                    {
                        ExpectedTrap = true;
                        ExpectedIPOffset = TotalEmitSize;
                        ExpectedStepCount = TotalEmitCount;
                    }
                }

                Emitter.BeginEmit();
                if (it.Op.Type == OperandType::T::None)
                    Emitter.Emit(it.Code);
                else
                    Emitter.Emit(it.Code, it.Op);

                Assert::IsTrue(
                    Emitter.EndEmit(HostAddress + TotalEmitSize,
                        CodeSize - TotalEmitSize, &ResultSize),
                    L"emit failed");

                TotalEmitCount++;
                TotalEmitSize += static_cast<uint32_t>(ResultSize);
            }

            if (!ExpectedTrap)
            {
                ExpectedIPOffset = TotalEmitSize;
                ExpectedStepCount = TotalEmitCount;
            }

            Logger::WriteMessage(
                Format("Emit size %d\n", TotalEmitSize).c_str());

            VMBytecodeInterpreter Interpreter(*Memory_.get());
            int ExecStepCount = Interpreter.Execute(Context, TotalEmitCount);

            Assert::AreEqual(ExecStepCount, ExpectedStepCount, L"instruction end unreachable");
            Assert::AreEqual<size_t>(Context.IP, PrevIP + ExpectedIPOffset, L"IP mismatch");
            Assert::AreEqual<uint32_t>(Context.ExceptionState, ExpectedExceptionState, L"exception state mismatch");

            bool IsStackOper64 = VMBytecodeInterpreter::IsStackOper64Bit(Context);

            std::tuple<uint32_t, VMStack*, StackState*, const wchar_t*> StackTestList[]
            {
                // PrevTop, Stack, StackName
                std::make_tuple(PrevStackTop, &Context.Stack, &ExpectedStackState, L"stack"),
                std::make_tuple(PrevArgumentStackTop, &Context.ArgumentStack, &ExpectedArgumentStackState, L"argument stack"),
                std::make_tuple(PrevLocalVarStackTop, &Context.LocalVariableStack, &ExpectedLocalVarStackState, L"localvar stack"),
            };

            uint64_t OutputCompareMask = 0;
            switch (ExpectedOutputResultSize)
            {
            case 0: break; // ignore
            case 1: OutputCompareMask = 0xff; break;
            case 2: OutputCompareMask = 0xffff; break;
            case 4: OutputCompareMask = 0xffffffff; break;
            case 8: OutputCompareMask = 0xffffffff'ffffffff; break;
            default:
                Assert::IsTrue(false, L"invalid parameter was specified");
            }

            for (auto& it : StackTestList)
            {
                uint32_t PrevTop = 0;
                VMStack* Stack = nullptr;
                StackState* StackState = nullptr;
                const wchar_t* StackName = L"";
                std::tie(PrevTop, Stack, StackState, StackName) = it;

                if (StackState->Invalid)
                    continue; // ignore state check

                // get current stack top first
                uint32_t CurrentStackTop = Stack->TopOffset();
                uint32_t ExpectedStackTop = PrevTop;

                if (IsStackOper64)
                {
                    using TStackElement = uint64_t;
                    int32_t ExpectedStackPushedBytes = StackState->PushedCount * sizeof(TStackElement);
                    TStackElement Element = 0;

                    for (auto PushedElementsIt = StackState->Pushed.rbegin();
                        PushedElementsIt != StackState->Pushed.rend();
                        ++PushedElementsIt)
                    {
                        // NOTE: this changes stack state
                        Assert::IsTrue(Stack->Pop(&Element));
                        Assert::AreEqual<TStackElement>(
                            Element, //Element & OutputCompareMask,
                            (*PushedElementsIt), //(*PushedElementsIt) & OutputCompareMask,
                            L"result mismatch");
                    }

                    ExpectedStackTop -= static_cast<uint32_t>(ExpectedStackPushedBytes);
                }
                else
                {
                    using TStackElement = uint32_t;
                    int32_t ExpectedStackPushedBytes = StackState->PushedCount * sizeof(TStackElement);
                    TStackElement Element = 0;

                    for (auto PushedElementsIt = StackState->Pushed.rbegin();
                        PushedElementsIt != StackState->Pushed.rend();
                        ++PushedElementsIt)
                    {
                        // NOTE: this changes stack state
                        Assert::IsTrue(Stack->Pop(&Element));
                        Assert::AreEqual<TStackElement>(
                            Element, //Element & OutputCompareMask, 
                            static_cast<TStackElement>(*PushedElementsIt), //static_cast<TStackElement>(*PushedElementsIt) & OutputCompareMask, 
                            L"result mismatch");
                    }

                    ExpectedStackTop -= static_cast<uint32_t>(ExpectedStackPushedBytes);
                }

                Assert::AreEqual(CurrentStackTop, ExpectedStackTop,
                    (std::wstring(StackName) + L" top mismatch").c_str());
            }

            ResultContext = Context;
        }


        template <
            typename T,
            typename U = T,
            typename = std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>,
            typename = std::enable_if_t<std::is_integral<U>::value || std::is_floating_point<U>::value>>
            void Test_LoadN_Op(
                const VMExecutionContext& InitialContext, 
                VMExecutionContext& ResultContext, 
                std::vector<T> LoadValues, 
                Opcode::T TestOp, 
                U OperandValue1, 
                uint32_t ExpectedOutputResultSize, 
                ExceptionState::T ExpectedExceptionState, 
                StackState ExpectedStackState, 
                StackState ExpectedArgumentStackState, 
                StackState ExpectedLocalVarStackState)
        {
            Opcode::T LoadOp = Opcode::T::Ldimm_I4;
            switch (sizeof(T))
            {
            case 1: LoadOp = Opcode::T::Ldimm_I1; break;
            case 2: LoadOp = Opcode::T::Ldimm_I2; break;
            case 4: LoadOp = Opcode::T::Ldimm_I4; break;
            case 8: LoadOp = Opcode::T::Ldimm_I8; break;
            default:
                Assert::IsTrue(false);
            }

            std::vector<EmitInfo> EmitInfos;
            for (auto& it : LoadValues)
            {
                EmitInfos.push_back(EmitInfo(false, LoadOp, OperandHelper(ToBinaryRepr(it))));
            }

            Operand Operand1 = OperandHelper(ToBinaryRepr(OperandValue1));
            EmitInfos.push_back(
                EmitInfo(ExpectedExceptionState != ExceptionState::T::None, TestOp, Operand1));

            DoSingleTest_New(InitialContext, ResultContext, EmitInfos, ExpectedOutputResultSize,
                ExpectedExceptionState, ExpectedStackState, ExpectedArgumentStackState, ExpectedLocalVarStackState);
        }

        template <
            typename T,
            typename U = T,
            typename = std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>,
            typename = std::enable_if_t<std::is_integral<U>::value || std::is_floating_point<U>::value>>
            void Test_LoadN_Op(
                std::vector<T> LoadValues,
                Opcode::T TestOp,
                U OperandValue1,
                uint32_t ExpectedOutputResultSize,
                ExceptionState::T ExpectedExceptionState,
                StackState ExpectedStackState,
                StackState ExpectedArgumentStackState,
                StackState ExpectedLocalVarStackState)
        {
            Test_LoadN_Op(ExecutionContextInitial_, ExecutionContext_,
                LoadValues, TestOp, OperandValue1, ExpectedOutputResultSize, ExpectedExceptionState, 
                ExpectedStackState, ExpectedArgumentStackState, ExpectedLocalVarStackState);
        }


        template <
            typename T,
            typename U = T,
            typename = std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>,
            typename = std::enable_if_t<std::is_integral<U>::value || std::is_floating_point<U>::value>>
            void Test_LoadImm1_Op(
                const VMExecutionContext& InitialContext, VMExecutionContext& ResultContext, 
                Opcode::T TestOp, T Value1, U ResultExpected, bool AssumeResultSignExtended)
        {
            using TBinaryRepr = decltype(ToBinaryRepr(ResultExpected));

            uint64_t Result64 = 0;
            if (AssumeResultSignExtended)
            {
                Result64 = static_cast<int64_t>(std::make_signed_t<TBinaryRepr>(ToBinaryRepr(ResultExpected)));
            }
            else
            {
                Result64 = static_cast<uint64_t>(std::make_unsigned_t<TBinaryRepr>(ToBinaryRepr(ResultExpected)));
            }

            Test_LoadN_Op(
                InitialContext, ResultContext,
                std::vector<T> { Value1, },
                TestOp, 0, sizeof(U),
                ExceptionState::T::None,
                StackState(std::vector<uint64_t> { Result64 }), // stack
                {}, // argument stack
                {}); // localvar stack
        }

        template <
            typename T,
            typename U = T,
            typename = std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>,
            typename = std::enable_if_t<std::is_integral<U>::value || std::is_floating_point<U>::value>>
            void Test_LoadImm1_Op(
                Opcode::T TestOp, T Value1, U ResultExpected, bool AssumeResultSignExtended)
        {
            Test_LoadImm1_Op(ExecutionContextInitial_, ExecutionContext_, TestOp, Value1, 
                ResultExpected, AssumeResultSignExtended);
        }

        template <
            typename T,
            typename U = T,
            typename = std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>,
            typename = std::enable_if_t<std::is_integral<U>::value || std::is_floating_point<U>::value>>
            void Test_LoadImm2_Op(
                const VMExecutionContext& InitialContext, VMExecutionContext& ResultContext,
                Opcode::T TestOp, T Value1, T Value2, U ResultExpected, bool AssumeResultSignExtended)
        {
            using TBinaryRepr = decltype(ToBinaryRepr(ResultExpected));

            uint64_t Result64 = 0;
            if (AssumeResultSignExtended)
            {
                Result64 = static_cast<int64_t>(std::make_signed_t<TBinaryRepr>(ToBinaryRepr(ResultExpected)));
            }
            else
            {
                Result64 = static_cast<uint64_t>(std::make_unsigned_t<TBinaryRepr>(ToBinaryRepr(ResultExpected)));
            }

            Test_LoadN_Op(
                InitialContext, ResultContext,
                std::vector<T> { Value1, Value2 },
                TestOp, 0, sizeof(U),
                ExceptionState::T::None,
                StackState(std::vector<uint64_t> { Result64 }), // stack
                {}, // argument stack
                {}); // localvar stack
        }

        template <
            typename T,
            typename U = T,
            typename = std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>,
            typename = std::enable_if_t<std::is_integral<U>::value || std::is_floating_point<U>::value>>
            void Test_LoadImm2_Op_Exception(
                const VMExecutionContext& InitialContext, VMExecutionContext& ResultContext,
                Opcode::T TestOp, T Value1, T Value2, ExceptionState::T ExpectedExceptionState)
        {
            Assert::AreNotEqual<uint32_t>(ExpectedExceptionState, ExceptionState::T::None, L"invalid parameter was specified");

            Test_LoadN_Op(
                InitialContext, ResultContext,
                std::vector<T> { Value1, Value2 },
                TestOp, 0, 0,
                ExpectedExceptionState,
                StackState::InvalidStackState(), // stack
                StackState::InvalidStackState(), // argument stack
                StackState::InvalidStackState()); // localvar stack
        }

        template <
            typename T,
            typename U = T,
            typename = std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>,
            typename = std::enable_if_t<std::is_integral<U>::value || std::is_floating_point<U>::value>>
            void Test_LoadImm2_Op_Exception(
                Opcode::T TestOp, T Value1, T Value2, ExceptionState::T ExpectedExceptionState)
        {
            Test_LoadImm2_Op_Exception(
                ExecutionContextInitial_, ExecutionContext_,
                TestOp, Value1, Value2, ExpectedExceptionState);
        }


        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>>
            void Test_NoLoad_Op(
                const VMExecutionContext& InitialContext, VMExecutionContext& ResultContext,
                Opcode::T TestOp, T OperandValue1, uint32_t ExpectedOutputResultSize, 
                ExceptionState::T ExpectedExceptionState,
                StackState ExpectedStackState, StackState ExpectedArgumentStackState, 
                StackState ExpectedLocalVarStackState)
        {
            Test_LoadN_Op(
                InitialContext, ResultContext,
                std::vector<uint32_t> { },
                TestOp, OperandValue1, ExpectedOutputResultSize, 
                ExpectedExceptionState,
                ExpectedStackState, // stack
                ExpectedArgumentStackState, // argument stack
                ExpectedLocalVarStackState); // localvar stack
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>>
            void Test_NoLoad_Op(
                Opcode::T TestOp, T OperandValue1, uint32_t ExpectedOutputResultSize,
                ExceptionState::T ExpectedExceptionState,
                StackState ExpectedStackState, StackState ExpectedArgumentStackState,
                StackState ExpectedLocalVarStackState)
        {
            Test_LoadN_Op(
                ExecutionContextInitial_, ExecutionContext_,
                std::vector<uint32_t> { },
                TestOp, OperandValue1, ExpectedOutputResultSize,
                ExpectedExceptionState,
                ExpectedStackState, // stack
                ExpectedArgumentStackState, // argument stack
                ExpectedLocalVarStackState); // localvar stack
        }

        template <
            typename T,
            typename U = T,
            typename = std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>,
            typename = std::enable_if_t<std::is_integral<U>::value || std::is_floating_point<U>::value>>
            void Test_LoadImm2_Op(Opcode::T TestOp, T Value1, T Value2, U ResultExpected, bool AssumeResultSignExtended)
        {
            Test_LoadImm2_Op(ExecutionContextInitial_, ExecutionContext_, 
                TestOp, Value1, Value2, ResultExpected, AssumeResultSignExtended);
        }

        TEST_METHOD(Inst_Nop)
        {
            Test_NoLoad_Op(Opcode::T::Nop, 0, 0, ExceptionState::T::None, {}, {}, {});
        }

        TEST_METHOD(Inst_Bp)
        {
            Test_NoLoad_Op(Opcode::T::Bp, 0, 0, ExceptionState::T::Breakpoint, {}, {}, {});
        }

        TEST_METHOD(Inst_Add)
        {
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Add_I4, 0x11223344, 0x44332211, 0x55555555, true);
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Add_U4, 0x11223344, 0x44332211, 0x55555555, false);

            Test_LoadImm2_Op<uint64_t>(Opcode::T::Add_I8, 0x11223344'55443322, 0x44332211'22334455, 0x55555555'77777777, true);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Add_U8, 0x11223344'55443322, 0x44332211'22334455, 0x55555555'77777777, false);

            Test_LoadImm2_Op<float>(Opcode::T::Add_F4, 123.456f, 654.321f, 123.456f + 654.321f, true);
            Test_LoadImm2_Op<double>(Opcode::T::Add_F8, 123.456, 654.321, 123.456 + 654.321, true);
        }

        TEST_METHOD(Inst_Sub)
        {
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Sub_I4, 0x11223344, 0x44332211, 0xccef1133, true);
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Sub_U4, 0x11223344, 0x44332211, 0xccef1133, false);

            Test_LoadImm2_Op<uint64_t>(Opcode::T::Sub_I8, 0x11223344'55443322, 0x44332211'22334455, 0xccef1133'3310eecd, true);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Sub_U8, 0x11223344'55443322, 0x44332211'22334455, 0xccef1133'3310eecd, false);

            Test_LoadImm2_Op<float>(Opcode::T::Sub_F4, 123.456f, 654.321f, 123.456f - 654.321f, true);
            Test_LoadImm2_Op<double>(Opcode::T::Sub_F8, 123.456, 654.321, 123.456 - 654.321, true);
        }

        TEST_METHOD(Inst_Mul)
        {
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Mul_I4, 0x1122, 0x3344, 0x36e5308, true);
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Mul_U4, 0x1122, 0x3344, 0x36e5308, false);

            Test_LoadImm2_Op<uint64_t>(Opcode::T::Mul_I8, 0x11223344, 0x55667788, 0x5b736a6'0117d820, true);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Mul_U8, 0x11223344, 0x55667788, 0x5b736a6'0117d820, false);

            Test_LoadImm2_Op<float>(Opcode::T::Mul_F4, 123.456f, 654.321f, 123.456f * 654.321f, true);
            Test_LoadImm2_Op<double>(Opcode::T::Mul_F8, 123.456, 654.321, 123.456 * 654.321, true);
        }

        TEST_METHOD(Inst_Mulh)
        {
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Mulh_I4, 0x11223344, 0x55667788, 0x5b736a6, true);
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Mulh_U4, 0x11223344, 0x55667788, 0x5b736a6, false);

            Test_LoadImm2_Op<uint64_t>(Opcode::T::Mulh_I8, 0x11223344'44332211, 0x44332211'11223344, 0x49081b6'07f13334, true);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Mulh_U8, 0x11223344'44332211, 0x44332211'11223344, 0x49081b6'07f13334, false);
        }

        TEST_METHOD(Inst_Div)
        {
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Div_I4, 0x44332211, 0x11223344, 3, true);
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Div_U4, 0x44332211, 0x11223344, 3, false);
            Test_LoadImm2_Op_Exception<uint32_t>(Opcode::Div_I4, 0x44332211, 0, ExceptionState::T::IntegerDivideByZero);
            Test_LoadImm2_Op_Exception<uint32_t>(Opcode::Div_U4, 0x44332211, 0, ExceptionState::T::IntegerDivideByZero);


            Test_LoadImm2_Op<uint64_t>(Opcode::T::Div_I8, 0x44332211'11223344, 0x11223344'44332211, 3, true);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Div_U8, 0x44332211'11223344, 0x11223344'44332211, 3, false);
            Test_LoadImm2_Op_Exception<uint64_t>(Opcode::Div_I8, 0x44332211'11223344, 0, ExceptionState::T::IntegerDivideByZero);
            Test_LoadImm2_Op_Exception<uint64_t>(Opcode::Div_U8, 0x44332211'11223344, 0, ExceptionState::T::IntegerDivideByZero);

            Test_LoadImm2_Op(Opcode::T::Div_F4, 123.456f, 654.321f, 123.456f / 654.321f, false);
            Test_LoadImm2_Op(Opcode::T::Div_F8, 123.456, 654.321, 123.456 / 654.321, false);
        }

        TEST_METHOD(Inst_Mod)
        {
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Mod_I4, 0x44332211, 0x11223344, 0x10cc8845, true);
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Mod_U4, 0x44332211, 0x11223344, 0x10cc8845, false);
            Test_LoadImm2_Op_Exception<uint32_t>(Opcode::Mod_I4, 0x44332211, 0, ExceptionState::T::IntegerDivideByZero);
            Test_LoadImm2_Op_Exception<uint32_t>(Opcode::Mod_U4, 0x44332211, 0, ExceptionState::T::IntegerDivideByZero);


            Test_LoadImm2_Op<uint64_t>(Opcode::T::Mod_I8, 0x44332211'11223344, 0x11223344'44332211, 0x10cc8844'4488cd11, true);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Mod_U8, 0x44332211'11223344, 0x11223344'44332211, 0x10cc8844'4488cd11, false);
            Test_LoadImm2_Op_Exception<uint64_t>(Opcode::Mod_I8, 0x44332211'11223344, 0, ExceptionState::T::IntegerDivideByZero);
            Test_LoadImm2_Op_Exception<uint64_t>(Opcode::Mod_U8, 0x44332211'11223344, 0, ExceptionState::T::IntegerDivideByZero);

            Test_LoadImm2_Op<float>(Opcode::T::Mod_F4, 654.321f, 123.456f, std::fmod(654.321f, 123.456f), true);
            Test_LoadImm2_Op<double>(Opcode::T::Mod_F8, 654.321, 123.456, std::fmod(654.321, 123.456), true);
        }

        TEST_METHOD(Inst_Shl)
        {
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Shl_I4, 0x44332211, 16, 0x22110000, true);
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Shl_U4, 0x44332211, 16, 0x22110000, false);

            Test_LoadImm2_Op<uint64_t>(Opcode::T::Shl_I8, 0x44332211'11223344, 32, 0x11223344'00000000, true);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Shl_U8, 0x44332211'11223344, 32, 0x11223344'00000000, false);
        }

        TEST_METHOD(Inst_Shr)
        {
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Shr_I4, 0x44332211, 16, 0x4433, true);
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Shr_U4, 0x44332211, 16, 0x4433, false);

            Test_LoadImm2_Op<uint64_t>(Opcode::T::Shr_I8, 0x44332211'11223344, 32, 0x44332211, true);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Shr_U8, 0x44332211'11223344, 32, 0x44332211, false);
        }

        TEST_METHOD(Inst_And)
        {
            Test_LoadImm2_Op<uint32_t>(Opcode::T::And_X4, 0x44332211, 0xff00ff00, 0x44002200, false);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::And_X8, 0x44332211'11223344, 0xff00ff00'ff00ff00, 0x44002200'11003300, false);
        }

        TEST_METHOD(Inst_Or)
        {
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Or_X4, 0x44002200, 0x00330011, 0x44332211, false);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Or_X8, 0x44002200'11003300, 0x00330011'00220044, 0x44332211'11223344, false);
        }

        TEST_METHOD(Inst_Xor)
        {
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Xor_X4, 0x44332211, 0xff00ff00, 0xbb33dd11, false);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Xor_X8, 0x44332211'11223344, 0xff00ff00'ff00ff00, 0xbb33dd11ee22cc44, false);
        }

        TEST_METHOD(Inst_Not)
        {
            Test_LoadImm1_Op<uint32_t>(Opcode::T::Not_X4, 0x44332211, ~0x44332211, false);
            Test_LoadImm1_Op<uint64_t>(Opcode::T::Not_X8, 0x44332211'11223344, ~0x44332211'11223344, false);
        }

        TEST_METHOD(Inst_Neg)
        {
            Test_LoadImm1_Op<uint32_t>(Opcode::T::Neg_I4, 0x44332211, ~0x44332211 + 1, true);
            Test_LoadImm1_Op<uint64_t>(Opcode::T::Neg_I8, 0x44332211'11223344, ~0x44332211'11223344 + 1, true);

            Test_LoadImm1_Op<float>(Opcode::T::Neg_F4, 123.456f, -123.456f, true);
            Test_LoadImm1_Op<double>(Opcode::T::Neg_F8, 123.456, -123.456, true);
        }

        TEST_METHOD(Inst_Abs)
        {
            Test_LoadImm1_Op<uint32_t>(Opcode::T::Abs_I4, ~0x44332211 + 1, 0x44332211, true);
            Test_LoadImm1_Op<uint64_t>(Opcode::T::Abs_I8, ~0x44332211'11223344 + 1, 0x44332211'11223344, true);

            Test_LoadImm1_Op<float>(Opcode::T::Abs_F4, -123.456f, 123.456f, true);
            Test_LoadImm1_Op<double>(Opcode::T::Abs_F8, -123.456, 123.456, true);
        }

        TEST_METHOD(Inst_Cvt2i)
        {
            Test_LoadImm1_Op<float, uint32_t>(Opcode::T::Cvt2i_F4_I4, 123.45f, 123, true);
            Test_LoadImm1_Op<float, uint64_t>(Opcode::T::Cvt2i_F4_I8, 123.45f, 123, true);

            Test_LoadImm1_Op<double, uint32_t>(Opcode::T::Cvt2i_F8_I4, 123.45, 123, true);
            Test_LoadImm1_Op<double, uint64_t>(Opcode::T::Cvt2i_F8_I8, 123.45, 123, true);
        }

        TEST_METHOD(Inst_Cvt2f)
        {
            Test_LoadImm1_Op<uint32_t, float>(Opcode::T::Cvt2f_I4_F4, 123, 123.f, true);
            Test_LoadImm1_Op<uint32_t, double>(Opcode::T::Cvt2f_I4_F8, 123, 123.0, true);

            Test_LoadImm1_Op<uint64_t, float>(Opcode::T::Cvt2f_I8_F4, 123, 123.f, true);
            Test_LoadImm1_Op<uint64_t, double>(Opcode::T::Cvt2f_I8_F8, 123, 123.0, true);
        }

        TEST_METHOD(Inst_Cvtff)
        {
            Test_LoadImm1_Op<float, double>(Opcode::T::Cvtff_F4_F8, 123.f, 123.0, true);
            Test_LoadImm1_Op<double, float>(Opcode::T::Cvtff_F8_F4, 123.0, 123.f, true);
        }

        TEST_METHOD(Inst_Cvt)
        {
            Test_LoadImm1_Op<uint8_t, uint32_t>(Opcode::T::Cvt_I1_I4, 0x81, 0xffffff81, true);
            Test_LoadImm1_Op<uint16_t, uint32_t>(Opcode::T::Cvt_I2_I4, 0x8001, 0xffff8001, true);
            Test_LoadImm1_Op<uint32_t, uint8_t>(Opcode::T::Cvt_I4_I1, 0xffffff81, 0x81, true);
            Test_LoadImm1_Op<uint32_t, uint16_t>(Opcode::T::Cvt_I4_I2, 0xffff8001, 0x8001, true);
            Test_LoadImm1_Op<uint32_t, uint64_t>(Opcode::T::Cvt_I4_I8, 0xffffff81, 0xffffffff'ffffff81, true);
            Test_LoadImm1_Op<uint64_t, uint32_t>(Opcode::T::Cvt_I8_I4, 0xffffffff'ffffff81, 0xffffff81, true);

            Test_LoadImm1_Op<uint8_t, uint32_t>(Opcode::T::Cvt_U1_U4, 0x81, 0x81, false);
            Test_LoadImm1_Op<uint16_t, uint32_t>(Opcode::T::Cvt_U2_U4, 0x8001, 0x8001, false);
            Test_LoadImm1_Op<uint32_t, uint8_t>(Opcode::T::Cvt_U4_U1, 0xffffff81, 0x81, false);
            Test_LoadImm1_Op<uint32_t, uint16_t>(Opcode::T::Cvt_U4_U2, 0xffffff81, 0xff81, false);
            Test_LoadImm1_Op<uint32_t, uint64_t>(Opcode::T::Cvt_U4_U8, 0xffffff81, 0x00000000'ffffff81, false);
            Test_LoadImm1_Op<uint64_t, uint32_t>(Opcode::T::Cvt_U8_U4, 0xffffffff'ffffff81, 0xffffff81, false);

            Test_LoadImm1_Op<uint8_t, uint8_t>(Opcode::T::Cvt_I1_U1, 0x81, 0x81, false);
            Test_LoadImm1_Op<uint16_t, uint16_t>(Opcode::T::Cvt_I2_U2, 0x8001, 0x8001, false);
            Test_LoadImm1_Op<uint32_t, uint32_t>(Opcode::T::Cvt_I4_U4, 0x80000001, 0x80000001, false);
            Test_LoadImm1_Op<uint64_t, uint64_t>(Opcode::T::Cvt_I8_U8, 0x80000000'00000001, 0x80000000'00000001, false);

            Test_LoadImm1_Op<uint8_t, uint8_t>(Opcode::T::Cvt_U1_I1, 0x81, 0x81, true);
            Test_LoadImm1_Op<uint16_t, uint16_t>(Opcode::T::Cvt_U2_I2, 0x8001, 0x8001, true);
            Test_LoadImm1_Op<uint32_t, uint32_t>(Opcode::T::Cvt_U4_I4, 0x80000001, 0x80000001, true);
            Test_LoadImm1_Op<uint64_t, uint64_t>(Opcode::T::Cvt_U8_I8, 0x80000000'00000001, 0x80000000'00000001, true);
        }

        TEST_METHOD(Inst_Ldimm)
        {
            //
            // normal test
            //

            Test_NoLoad_Op<uint8_t>(Opcode::T::Ldimm_I1, 0x81, 0, ExceptionState::T::None,
                StackState(std::vector<uint64_t> { 0xffffffff'ffffff81 }),
                StackState(), StackState());

            Test_NoLoad_Op<uint16_t>(Opcode::T::Ldimm_I2, 0x8001, 0, ExceptionState::T::None,
                StackState(std::vector<uint64_t> { 0xffffffff'ffff8001 }),
                StackState(), StackState());

            Test_NoLoad_Op<uint32_t>(Opcode::T::Ldimm_I4, 0x80000001, 0, ExceptionState::T::None,
                StackState(std::vector<uint64_t> { 0xffffffff'80000001 }),
                StackState(), StackState());

            Test_NoLoad_Op<uint64_t>(Opcode::T::Ldimm_I8, 0x80000000'00000001, 0, ExceptionState::T::None,
                StackState(std::vector<uint64_t> { 0x80000000'00000001 }),
                StackState(), StackState());


            //
            // stack overflow test
            //

            VMExecutionContext InitialContext = ExecutionContextInitial_;
            VMExecutionContext ReturnContext;
            InitialContext.Stack.SetTopOffset(0); // end of stack

            Test_NoLoad_Op<uint8_t>(InitialContext, ReturnContext,
                Opcode::T::Ldimm_I1, 0x81, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());

            Test_NoLoad_Op<uint16_t>(InitialContext, ReturnContext,
                Opcode::T::Ldimm_I2, 0x8001, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());

            Test_NoLoad_Op<uint32_t>(InitialContext, ReturnContext,
                Opcode::T::Ldimm_I4, 0x80000001, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());

            Test_NoLoad_Op<uint64_t>(InitialContext, ReturnContext,
                Opcode::T::Ldimm_I8, 0x80000000'00000001, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
        }

        //Ldarg,
        //Ldvar,
        //Starg,
        //Stvar,

        TEST_METHOD(Inst_Dup)
        {
            Test_LoadN_Op<uint32_t>(
                std::vector<uint32_t> { 0x87654321 },
                Opcode::T::Dup, 0, 0, ExceptionState::T::None,
                StackState(std::vector<uint64_t> { 0xffffffff'87654321, 0xffffffff'87654321 }),
                StackState(), StackState());

            // stack overflow test
            VMExecutionContext InitialContext = ExecutionContextInitial_;
            VMExecutionContext ReturnContext;
            InitialContext.Stack.SetTopOffset(0); // end of stack

            Test_LoadN_Op<uint32_t>(
                InitialContext, ReturnContext,
                std::vector<uint32_t> { },
                Opcode::T::Dup, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
        }

        TEST_METHOD(Inst_Dup2)
        {
            Test_LoadN_Op<uint32_t>(
                std::vector<uint32_t> { 0x87654321, 0x12345678 },
                Opcode::T::Dup2, 0, 0, ExceptionState::T::None,
                StackState(std::vector<uint64_t>
                {
                    0xffffffff'87654321, 0x00000000'12345678,
                    0xffffffff'87654321, 0x00000000'12345678,
                }),
                StackState(), StackState());

            // stack overflow test
            VMExecutionContext InitialContext = ExecutionContextInitial_;
            VMExecutionContext ReturnContext;
            InitialContext.Stack.SetTopOffset(0); // end of stack

            Test_LoadN_Op<uint32_t>(
                InitialContext, ReturnContext,
                std::vector<uint32_t> { },
                Opcode::T::Dup2, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
        }

        TEST_METHOD(Inst_Xch)
        {
            Test_LoadN_Op<uint32_t>(
                std::vector<uint32_t> { 0x87654321, 0x12345678 },
                Opcode::T::Xch, 0, 0, ExceptionState::T::None,
                StackState(std::vector<uint64_t>
                {
                    0x00000000'12345678, 0xffffffff'87654321,
                }),
                StackState(), StackState());

            // stack overflow test

            Test_LoadN_Op<uint32_t>(
                std::vector<uint32_t> { 0x87654321, },
                Opcode::T::Xch, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
        }

        //Ldvarp,
        //Ldargp,
        //Ldpv_X1,
        //Ldpv_X2,
        //Ldpv_X4,
        //Ldpv_X8,
        //Stpv_X1,
        //Stpv_X2,
        //Stpv_X4,
        //Stpv_X8,
        //Ppcpy,
        //Pvfil_X1,
        //Pvfil_X2,
        //Pvfil_X4,
        //Pvfil_X8,
        //
        //Initarg,
        //Arg,
        //Var,

        TEST_METHOD(Inst_Dcv)
        {
            Test_LoadN_Op<uint32_t>(
                std::vector<uint32_t> { 0x87654321, 0x12345678 },
                Opcode::T::Dcv, 0, 0, ExceptionState::T::None,
                StackState(std::vector<uint64_t> { 0xffffffff'87654321, }),
                StackState(), StackState());

            Test_LoadN_Op<uint32_t>(
                std::vector<uint32_t> { 0x87654321, },
                Opcode::T::Dcv, 0, 0, ExceptionState::T::None,
                StackState(std::vector<uint64_t> { }),
                StackState(), StackState());

            // stack overflow test

            Test_LoadN_Op<uint32_t>(
                std::vector<uint32_t> { },
                Opcode::T::Dcv, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
        }

        TEST_METHOD(Inst_Dcvn)
        {
            Test_LoadN_Op<uint32_t>(
                std::vector<uint32_t> { 0x87654321, 0x12345678, 2 },
                Opcode::T::Dcvn, 0, 0, ExceptionState::T::None,
                StackState(std::vector<uint64_t> { }),
                StackState(), StackState());

            Test_LoadN_Op<uint32_t>(
                std::vector<uint32_t> { 0x87654321, 0x12345678, 1 },
                Opcode::T::Dcvn, 0, 0, ExceptionState::T::None,
                StackState(std::vector<uint64_t> { 0xffffffff'87654321, }),
                StackState(), StackState());

            Test_LoadN_Op<uint32_t>(
                std::vector<uint32_t> { 0x87654321, 0x12345678, 0 },
                Opcode::T::Dcvn, 0, 0, ExceptionState::T::None,
                StackState(std::vector<uint64_t> { 0xffffffff'87654321, 0x00000000'12345678 }),
                StackState(), StackState());

            // stack overflow test

            Test_LoadN_Op<uint32_t>(
                std::vector<uint32_t> { 0x87654321, 0x12345678, 3 },
                Opcode::T::Dcvn, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());

            Test_LoadN_Op<uint32_t>(
                std::vector<uint32_t> { 0x12345678, 2 },
                Opcode::T::Dcvn, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());

            Test_LoadN_Op<uint32_t>(
                std::vector<uint32_t> { 1 },
                Opcode::T::Dcvn, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());

            Test_LoadN_Op<uint32_t>(
                std::vector<uint32_t> { },
                Opcode::T::Dcvn, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
        }

        TEST_METHOD(Inst_Test_e)
        {
            // equal
            int32_t Expected = 1;
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Test_e_I4, 0x44332211, 0x44332211, Expected, true);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Test_e_I8, 0x88776655'44332211, 0x88776655'44332211, Expected, true);
            Test_LoadImm2_Op<float>(Opcode::T::Test_e_F4, 123.456f, 123.456f, Expected, true);
            Test_LoadImm2_Op<double>(Opcode::T::Test_e_F8, 123.456, 123.456, Expected, true);

            // not equal
            Expected = 0;
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Test_e_I4, 0x44332211, 0x44332211 + 1, Expected, true);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Test_e_I8, 0x88776655'44332211, 0x88776655'44332211 + 1, Expected, true);
            Test_LoadImm2_Op<float>(Opcode::T::Test_e_F4, 123.456f, 123.456f + 0.00001f, Expected, true);
            Test_LoadImm2_Op<double>(Opcode::T::Test_e_F8, 123.456, 123.456 + 0.00001, Expected, true);

            // stack overflow

            Test_NoLoad_Op(
                Opcode::T::Test_e_I4, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
            Test_NoLoad_Op(
                Opcode::T::Test_e_I8, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
            Test_NoLoad_Op(
                Opcode::T::Test_e_F4, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
            Test_NoLoad_Op(
                Opcode::T::Test_e_F8, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
        }

        TEST_METHOD(Inst_Test_ne)
        {
            // equal
            int32_t Expected = 0;
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Test_ne_I4, 0x44332211, 0x44332211, Expected, true);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Test_ne_I8, 0x88776655'44332211, 0x88776655'44332211, Expected, true);
            Test_LoadImm2_Op<float>(Opcode::T::Test_ne_F4, 123.456f, 123.456f, Expected, true);
            Test_LoadImm2_Op<double>(Opcode::T::Test_ne_F8, 123.456, 123.456, Expected, true);

            // not equal
            Expected = 1;
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Test_ne_I4, 0x44332211, 0x44332211 + 1, Expected, true);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Test_ne_I8, 0x88776655'44332211, 0x88776655'44332211 + 1, Expected, true);
            Test_LoadImm2_Op<float>(Opcode::T::Test_ne_F4, 123.456f, 123.456f + 0.00001f, Expected, true);
            Test_LoadImm2_Op<double>(Opcode::T::Test_ne_F8, 123.456, 123.456 + 0.00001, Expected, true);


            // stack overflow

            Test_NoLoad_Op(
                Opcode::T::Test_ne_I4, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
            Test_NoLoad_Op(
                Opcode::T::Test_ne_I8, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
            Test_NoLoad_Op(
                Opcode::T::Test_ne_F4, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
            Test_NoLoad_Op(
                Opcode::T::Test_ne_F8, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
        }

        TEST_METHOD(Inst_Test_le)
        {
            // equal
            int32_t Expected = 1;
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Test_le_I4, 0x44332211, 0x44332211, Expected, true);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Test_le_I8, 0x88776655'44332211, 0x88776655'44332211, Expected, true);
            Test_LoadImm2_Op<float>(Opcode::T::Test_le_F4, 123.456f, 123.456f, Expected, true);
            Test_LoadImm2_Op<double>(Opcode::T::Test_le_F8, 123.456, 123.456, Expected, true);

            // less
            Expected = 1;
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Test_le_I4, 0x44332211, 0x44332211 + 1, Expected, true);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Test_le_I8, 0x88776655'44332211, 0x88776655'44332211 + 1, Expected, true);
            Test_LoadImm2_Op<float>(Opcode::T::Test_le_F4, 123.456f, 123.456f + 0.00001f, Expected, true);
            Test_LoadImm2_Op<double>(Opcode::T::Test_le_F8, 123.456, 123.456 + 0.00001, Expected, true);

            // neither less nor equal
            Expected = 0;
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Test_le_I4, 0x44332211, 0x44332211 - 1, Expected, true);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Test_le_I8, 0x88776655'44332211, 0x88776655'44332211 - 1, Expected, true);
            Test_LoadImm2_Op<float>(Opcode::T::Test_le_F4, 123.456f, 123.456f - 0.00001f, Expected, true);
            Test_LoadImm2_Op<double>(Opcode::T::Test_le_F8, 123.456, 123.456 - 0.00001, Expected, true);

            // stack overflow

            Test_NoLoad_Op(
                Opcode::T::Test_le_I4, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
            Test_NoLoad_Op(
                Opcode::T::Test_le_I8, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
            Test_NoLoad_Op(
                Opcode::T::Test_le_F4, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
            Test_NoLoad_Op(
                Opcode::T::Test_le_F8, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
        }

        TEST_METHOD(Inst_Test_ge)
        {
            // equal
            int32_t Expected = 1;
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Test_ge_I4, 0x44332211, 0x44332211, Expected, true);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Test_ge_I8, 0x88776655'44332211, 0x88776655'44332211, Expected, true);
            Test_LoadImm2_Op<float>(Opcode::T::Test_ge_F4, 123.456f, 123.456f, Expected, true);
            Test_LoadImm2_Op<double>(Opcode::T::Test_ge_F8, 123.456, 123.456, Expected, true);

            // greater
            Expected = 1;
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Test_ge_I4, 0x44332211, 0x44332211 - 1, Expected, true);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Test_ge_I8, 0x88776655'44332211, 0x88776655'44332211 - 1, Expected, true);
            Test_LoadImm2_Op<float>(Opcode::T::Test_ge_F4, 123.456f, 123.456f - 0.00001f, Expected, true);
            Test_LoadImm2_Op<double>(Opcode::T::Test_ge_F8, 123.456, 123.456 - 0.00001, Expected, true);

            // neither greater nor equal
            Expected = 0;
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Test_ge_I4, 0x44332211, 0x44332211 + 1, Expected, true);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Test_ge_I8, 0x88776655'44332211, 0x88776655'44332211 + 1, Expected, true);
            Test_LoadImm2_Op<float>(Opcode::T::Test_ge_F4, 123.456f, 123.456f + 0.00001f, Expected, true);
            Test_LoadImm2_Op<double>(Opcode::T::Test_ge_F8, 123.456, 123.456 + 0.00001, Expected, true);

            // stack overflow

            Test_NoLoad_Op(
                Opcode::T::Test_ge_I4, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
            Test_NoLoad_Op(
                Opcode::T::Test_ge_I8, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
            Test_NoLoad_Op(
                Opcode::T::Test_ge_F4, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
            Test_NoLoad_Op(
                Opcode::T::Test_ge_F8, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
        }

        TEST_METHOD(Inst_Test_l)
        {
            // equal
            int32_t Expected = 0;
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Test_l_I4, 0x44332211, 0x44332211, Expected, true);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Test_l_I8, 0x88776655'44332211, 0x88776655'44332211, Expected, true);
            Test_LoadImm2_Op<float>(Opcode::T::Test_l_F4, 123.456f, 123.456f, Expected, true);
            Test_LoadImm2_Op<double>(Opcode::T::Test_l_F8, 123.456, 123.456, Expected, true);

            // less
            Expected = 1;
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Test_l_I4, 0x44332211, 0x44332211 + 1, Expected, true);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Test_l_I8, 0x88776655'44332211, 0x88776655'44332211 + 1, Expected, true);
            Test_LoadImm2_Op<float>(Opcode::T::Test_l_F4, 123.456f, 123.456f + 0.00001f, Expected, true);
            Test_LoadImm2_Op<double>(Opcode::T::Test_l_F8, 123.456, 123.456 + 0.00001, Expected, true);

            // neither less nor equal
            Expected = 0;
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Test_l_I4, 0x44332211, 0x44332211 - 1, Expected, true);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Test_l_I8, 0x88776655'44332211, 0x88776655'44332211 - 1, Expected, true);
            Test_LoadImm2_Op<float>(Opcode::T::Test_l_F4, 123.456f, 123.456f - 0.00001f, Expected, true);
            Test_LoadImm2_Op<double>(Opcode::T::Test_l_F8, 123.456, 123.456 - 0.00001, Expected, true);

            // stack overflow

            Test_NoLoad_Op(
                Opcode::T::Test_l_I4, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
            Test_NoLoad_Op(
                Opcode::T::Test_l_I8, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
            Test_NoLoad_Op(
                Opcode::T::Test_l_F4, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
            Test_NoLoad_Op(
                Opcode::T::Test_l_F8, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
        }

        TEST_METHOD(Inst_Test_g)
        {
            // equal
            int32_t Expected = 0;
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Test_g_I4, 0x44332211, 0x44332211, Expected, true);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Test_g_I8, 0x88776655'44332211, 0x88776655'44332211, Expected, true);
            Test_LoadImm2_Op<float>(Opcode::T::Test_g_F4, 123.456f, 123.456f, Expected, true);
            Test_LoadImm2_Op<double>(Opcode::T::Test_g_F8, 123.456, 123.456, Expected, true);

            // greater
            Expected = 1;
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Test_g_I4, 0x44332211, 0x44332211 - 1, Expected, true);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Test_g_I8, 0x88776655'44332211, 0x88776655'44332211 - 1, Expected, true);
            Test_LoadImm2_Op<float>(Opcode::T::Test_g_F4, 123.456f, 123.456f - 0.00001f, Expected, true);
            Test_LoadImm2_Op<double>(Opcode::T::Test_g_F8, 123.456, 123.456 - 0.00001, Expected, true);

            // neither greater nor equal
            Expected = 0;
            Test_LoadImm2_Op<uint32_t>(Opcode::T::Test_g_I4, 0x44332211, 0x44332211 + 1, Expected, true);
            Test_LoadImm2_Op<uint64_t>(Opcode::T::Test_g_I8, 0x88776655'44332211, 0x88776655'44332211 + 1, Expected, true);
            Test_LoadImm2_Op<float>(Opcode::T::Test_g_F4, 123.456f, 123.456f + 0.00001f, Expected, true);
            Test_LoadImm2_Op<double>(Opcode::T::Test_g_F8, 123.456, 123.456 + 0.00001, Expected, true);

            // stack overflow

            Test_NoLoad_Op(
                Opcode::T::Test_g_I4, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
            Test_NoLoad_Op(
                Opcode::T::Test_g_I8, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
            Test_NoLoad_Op(
                Opcode::T::Test_g_F4, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
            Test_NoLoad_Op(
                Opcode::T::Test_g_F8, 0, 0, ExceptionState::T::StackOverflow,
                StackState(true),
                StackState(), StackState());
        }

    private:
        std::unique_ptr<VMMemoryManager> Memory_;
        VMExecutionContext ExecutionContext_;
        VMExecutionContext ExecutionContextInitial_;

        GuestMemory GuestCode_;
        GuestMemory GuestStack_;
        GuestMemory GuestShadowStack_;
        GuestMemory GuestLocalVarStack_;
        GuestMemory GuestArgumentStack_;

        const uint32_t SizeOfBpInst = 1;
    };

    TEST_CLASS(IntegerTest)
    {
    public:
        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value>>
            struct Testcase
        {
            Integer<T> v1;
            Integer<T> v2;
            Integer<T> ResultExpected;
            uint8_t StateExpected;
        };

        enum class TestType : uint32_t
        {
            Equ,	// a == b
            Neq,	// a != b

            Equ2,	// a == b (with state)
            Neq2,	// a != b (with state)

            Add,    // a + b
            Sub,    // a - b

            Mul,    // a * b
            Div,    // a / b
            Rem,    // a % b

            Shl,    // a << b
            Shr,    // a >> b

            And,    // a & b
            Or,     // a | b
            Xor,    // a ^ b

            Neg,    // -a
            Not,    // ~a
            Inc,    // a++, ++a
            Dec,    // a--, --a
        };

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value>>
        T Shr_Signed_Helper(T v1, T v2)
        {
            T vr = 0;
            constexpr const T signbit = static_cast<T>(1) << ((sizeof(T) << 3) - 1);

            if (v2 >= (sizeof(T) << 3))
            {
                vr = (v1 & signbit) ? ~0 : 0;
            }
            else
            {
                vr = v1 >> v2;
                if (v1 & signbit)
                    vr |= ~(static_cast<T>(~0) >> v2);
            }
            return vr;
        };

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value>>
        void DoTest(TestType Type, const Testcase<T> Testcases[], size_t Count)
        {
            T Bitwidth = sizeof(T);

            for (size_t i = 0; i < Count; i++)
            {
                Testcase<T> Entry = Testcases[i];
                std::vector<Integer<T>> Results;

                switch (Type)
                {
                case TestType::Equ:	   // a == b
                {
                    Results = {
                        Entry.v1 == Entry.v2,
                        Entry.v2 == Entry.v1,
                    };
                    break;
                }

                case TestType::Neq:	   // a != b
                {
                    Results = {
                        Entry.v1 != Entry.v2,
                        Entry.v2 != Entry.v1,
                    };
                    break;
                }

                case TestType::Equ2:	  // a == b (with state)
                {
                    Results = {
                        Entry.v1.Equal(Entry.v2, true),
                        Entry.v2.Equal(Entry.v1, true),
                    };
                    break;
                }

                case TestType::Neq2:	  // a != b (with state)
                {
                    Results = {
                        Entry.v1.NotEqual(Entry.v2, true),
                        Entry.v2.NotEqual(Entry.v1, true),
                    };
                    break;
                }

                case TestType::Add:    // a + b
                {
                    Results = {
                        Entry.v1 + Entry.v2,
                        Entry.v2 + Entry.v1,
                        Integer<T>(Entry.v1) += Entry.v2,
                        Integer<T>(Entry.v2) += Entry.v1,
                    };
                    break;
                }
                case TestType::Sub:    // a - b
                {
                    Results = {
                        Entry.v1 - Entry.v2,
                        Integer<T>(Entry.v1) -= Entry.v2,
                    };
                    break;
                }
                case TestType::Mul:    // a * b
                {
                    Results = {
                        Entry.v1 * Entry.v2,
                        Entry.v2 * Entry.v1,
                        Integer<T>(Entry.v1) *= Entry.v2,
                        Integer<T>(Entry.v2) *= Entry.v1,
                    };
                    break;
                }
                case TestType::Div:    // a / b
                {
                    Results = {
                        Entry.v1 / Entry.v2,
                        Integer<T>(Entry.v1) /= Entry.v2,
                    };
                    break;
                }
                case TestType::Rem:    // a % b
                {
                    Results = {
                        Entry.v1 % Entry.v2,
                        Integer<T>(Entry.v1) %= Entry.v2,
                    };
                    break;
                }
                case TestType::Shl:    // a << b
                {
                    Results = {
                        Entry.v1 << Entry.v2,
                        Integer<T>(Entry.v1) <<= Entry.v2,
                    };
                    break;
                }
                case TestType::Shr:    // a >> b
                {
                    Results = {
                        Entry.v1 >> Entry.v2,
                        Integer<T>(Entry.v1) >>= Entry.v2,
                    };
                    break;
                }
                case TestType::And:    // a & b
                {
                    Results = {
                        Entry.v1 & Entry.v2,
                        Entry.v2 & Entry.v1,
                        Integer<T>(Entry.v1) &= Entry.v2,
                        Integer<T>(Entry.v2) &= Entry.v1,
                    };
                    break;
                }
                case TestType::Or:     // a | b
                {
                    Results = {
                        Entry.v1 | Entry.v2,
                        Entry.v2 | Entry.v1,
                        Integer<T>(Entry.v1) |= Entry.v2,
                        Integer<T>(Entry.v2) |= Entry.v1,
                    };
                    break;
                }
                case TestType::Xor:    // a ^ b
                {
                    Results = {
                        Entry.v1 ^ Entry.v2,
                        Entry.v2 ^ Entry.v1,
                        Integer<T>(Entry.v1) ^= Entry.v2,
                        Integer<T>(Entry.v2) ^= Entry.v1,
                    };
                    break;
                }
                case TestType::Neg:    // -a
                {
                    Results = {
                        -Entry.v1,
                    };
                    break;
                }
                case TestType::Not:    // ~a
                {
                    Results = {
                        ~Entry.v1,
                    };
                    break;
                }
                case TestType::Inc:    // a++, ++a
                {
                    Results = {
                        (Integer<T>(Entry.v1)++),
                        (++Integer<T>(Entry.v1)),
                    };
                    break;
                }
                case TestType::Dec:    // a--, --a
                {
                    Results = {
                        (Integer<T>(Entry.v1)--),
                        (--Integer<T>(Entry.v1)),
                    };
                    break;
                }
                default:
                    Assert::IsTrue(false, L"invalid test type");
                }

                for (size_t j = 0; j < Results.size(); j++)
                {
                    auto& it = Results[j];
                    Assert::AreEqual(Entry.ResultExpected.Value(), it.Value(), 
                        Format(L"result value mismatch on testcase %zd, result_item %zd", i, j).c_str());
                    Assert::AreEqual(Entry.StateExpected, it.State(),
                        Format(L"result state mismatch on testcase %zd, result_item %zd", i, j).c_str());
                }
            }
        }

        template <
            typename T,
            size_t Count,
            typename = std::enable_if_t<std::is_integral<T>::value>>
            void DoTest(TestType Type, const Testcase<T> (&Testcases)[Count])
        {
            DoTest(Type, Testcases, Count);
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value>>
            void DoTest(TestType Type, const std::vector<Testcase<T>>& Testcases)
        {
            DoTest(Type, Testcases.data(), Testcases.size());
        }


#define C_PUSH_WARNING_IGNORE		4307 4146
#include "push_warnings.h"
        template <
            typename T,
            std::enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value, bool> = true>
            void DoMultipleTest()
        {
            using TInt = typename T;
            using TUInt = typename std::make_unsigned_t<TInt>;
            using TSInt = typename std::make_signed_t<TInt>;

            constexpr const TUInt imin = std::numeric_limits<TInt>::min();
            constexpr const TUInt imax = std::numeric_limits<TInt>::max();
            constexpr const TUInt bitwidth = sizeof(TInt) << 3;
            constexpr const TUInt signbit = static_cast<TUInt>(1) << (bitwidth - 1);
            constexpr const TUInt fullmask = ~0;

            constexpr const uint8_t s_inv = Integer<TInt>::StateFlags::Invalid;
            constexpr const uint8_t s_ovf = Integer<TInt>::StateFlags::Overflow;
            constexpr const uint8_t s_div = Integer<TInt>::StateFlags::DivideByZero;

            Integer<TInt> nan;

            DoTest(TestType::Equ, std::vector<Testcase<TInt>>
            {
                // v1, v2, result_expected, state_expected

                { TInt(1), TInt(1), true, 0, },
                { TInt(1), TInt(2), false, 0, },
                { nan, TInt(1), false, 0, },

                { BaseInteger<TInt>(1, s_inv), BaseInteger<TInt>(1, s_inv), false, 0, },
                { BaseInteger<TInt>(1, s_inv), BaseInteger<TInt>(2, s_inv), false, 0, },
                { nan, nan, false, 0, },
            });

            DoTest(TestType::Neq, std::vector<Testcase<TInt>>
            {
                // v1, v2, result_expected, state_expected

                { TInt(1), TInt(1), false, 0, },
                { TInt(1), TInt(2), true, 0, },
                { nan, TInt(1), true, 0, },

                { BaseInteger<TInt>(1, s_inv), BaseInteger<TInt>(1, s_inv), true, 0, },
                { BaseInteger<TInt>(1, s_inv), BaseInteger<TInt>(2, s_inv), true, 0, },
                { nan, nan, true, 0, },
            });

            DoTest(TestType::Equ2, std::vector<Testcase<TInt>>
            {
                // v1, v2, result_expected, state_expected

                { TInt(1), TInt(1), true, 0, },
                { TInt(1), TInt(2), false, 0, },
                { nan, TInt(1), false, 0, },

                { BaseInteger<TInt>(1, s_inv), BaseInteger<TInt>(1, s_inv), true, 0, },
                { BaseInteger<TInt>(1, s_inv), BaseInteger<TInt>(2, s_inv), true, 0, },
                { nan, nan, true, 0, },
            });

            DoTest(TestType::Neq2, std::vector<Testcase<TInt>>
            {
                // v1, v2, result_expected, state_expected

                { TInt(1), TInt(1), false, 0, },
                { TInt(1), TInt(2), true, 0, },
                { nan, TInt(1), true, 0, },

                { BaseInteger<TInt>(1, s_inv), BaseInteger<TInt>(1, s_inv), false, 0, },
                { BaseInteger<TInt>(1, s_inv), BaseInteger<TInt>(2, s_inv), false, 0, },
                { nan, nan, false, 0, },
            });

            DoTest(TestType::Add, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1, _v2)        TInt(TUInt(_v1) + TUInt(_v2))
                // v1, v2, result_expected, state_expected

                { nan, 1, nan, s_inv, },

                { TInt(1), TInt(2), OpExpected(1, 2), 0, },
                { TInt(1), TInt(imax), OpExpected(1, imax), s_ovf, },
                { TInt(-1), TInt(imin), OpExpected(-1, imin), s_ovf, },

                { TInt(imin), TInt(-3), OpExpected(imin, -3), s_ovf, },
                { TInt(imin), TInt(-2), OpExpected(imin, -2), s_ovf, },
                { TInt(imin), TInt(-1), OpExpected(imin, -1), s_ovf, },
                { TInt(imin), TInt(1), OpExpected(imin, 1), 0, },
                { TInt(imin), TInt(2), OpExpected(imin, 2), 0, },
                { TInt(imin), TInt(3), OpExpected(imin, 3), 0, },

                { TInt(imin), TInt(imax), OpExpected(imin, imax), 0, },
                { TInt(imax), TInt(imin), OpExpected(imax, imin), 0, },

                { TInt(imax), TInt(-3), OpExpected(imax, -3), 0, },
                { TInt(imax), TInt(-2), OpExpected(imax, -2), 0, },
                { TInt(imax), TInt(-1), OpExpected(imax, -1), 0, },
                { TInt(imax), TInt(1), OpExpected(imax, 1), s_ovf, },
                { TInt(imax), TInt(2), OpExpected(imax, 2), s_ovf, },
                { TInt(imax), TInt(3), OpExpected(imax, 3), s_ovf, },

                { TInt(imin), TInt(imax - 3), OpExpected(imin, imax - 3), 0, },
                { TInt(imin), TInt(imax - 2), OpExpected(imin, imax - 2), 0, },
                { TInt(imin), TInt(imax - 1), OpExpected(imin, imax - 1), 0, },

                { TInt(imax), TInt(imin + 1), OpExpected(imax, imin + 1), 0, },
                { TInt(imax), TInt(imin + 2), OpExpected(imax, imin + 2), 0, },
                { TInt(imax), TInt(imin + 3), OpExpected(imax, imin + 3), 0, },

                #undef OpExpected
            });

            DoTest(TestType::Sub, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1, _v2)        TInt(TUInt(_v1) - TUInt(_v2))
                // v1, v2, result_expected, state_expected

                { nan, 1, nan, s_inv, },

                { TInt(1), TInt(2), OpExpected(1, 2), 0, },
                { TInt(1), TInt(imax), OpExpected(1, imax), 0, },
                { TInt(-1), TInt(imin), OpExpected(-1, imin), 0, },

                { TInt(imin), TInt(-3), OpExpected(imin, -3), 0, },
                { TInt(imin), TInt(-2), OpExpected(imin, -2), 0, },
                { TInt(imin), TInt(-1), OpExpected(imin, -1), 0, },
                { TInt(imin), TInt(1), OpExpected(imin, 1), s_ovf, },
                { TInt(imin), TInt(2), OpExpected(imin, 2), s_ovf, },
                { TInt(imin), TInt(3), OpExpected(imin, 3), s_ovf, },

                { TInt(imin), TInt(imax), OpExpected(imin, imax), s_ovf, },
                { TInt(imax), TInt(imin), OpExpected(imax, imin), s_ovf, },

                { TInt(imax), TInt(-3), OpExpected(imax, -3), s_ovf, },
                { TInt(imax), TInt(-2), OpExpected(imax, -2), s_ovf, },
                { TInt(imax), TInt(-1), OpExpected(imax, -1), s_ovf, },
                { TInt(imax), TInt(1), OpExpected(imax, 1), 0, },
                { TInt(imax), TInt(2), OpExpected(imax, 2), 0, },
                { TInt(imax), TInt(3), OpExpected(imax, 3), 0, },

                { TInt(imin), TInt(imax - 3), OpExpected(imin, imax - 3), s_ovf, },
                { TInt(imin), TInt(imax - 2), OpExpected(imin, imax - 2), s_ovf, },
                { TInt(imin), TInt(imax - 1), OpExpected(imin, imax - 1), s_ovf, },

                { TInt(imax), TInt(imin + 1), OpExpected(imax, imin + 1), s_ovf, },
                { TInt(imax), TInt(imin + 2), OpExpected(imax, imin + 2), s_ovf, },
                { TInt(imax), TInt(imin + 3), OpExpected(imax, imin + 3), s_ovf, },

                #undef OpExpected
            });

            DoTest(TestType::Mul, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1, _v2)        TInt(TUInt(_v1) * TUInt(_v2))
                // v1, v2, result_expected, state_expected

                { nan, 1, nan, s_inv, },

                { TInt(1), TInt(2), OpExpected(1, 2), 0, },
                { TInt(1), TInt(imax), OpExpected(1, imax), 0, },
                { TInt(-1), TInt(imin), OpExpected(-1, imin), s_ovf, },

                { TInt(imin), TInt(-3), OpExpected(imin, -3), s_ovf, },
                { TInt(imin), TInt(-2), OpExpected(imin, -2), s_ovf, },
                { TInt(imin), TInt(-1), OpExpected(imin, -1), s_ovf, },
                { TInt(imin), TInt(1), OpExpected(imin, 1), 0, },
                { TInt(imin), TInt(2), OpExpected(imin, 2), s_ovf, },
                { TInt(imin), TInt(3), OpExpected(imin, 3), s_ovf, },

                { TInt(imin), TInt(imax), OpExpected(imin, imax), s_ovf, },
                { TInt(imax), TInt(imin), OpExpected(imax, imin), s_ovf, },

                { TInt(imax), TInt(-3), OpExpected(imax, -3), s_ovf, },
                { TInt(imax), TInt(-2), OpExpected(imax, -2), s_ovf, },
                { TInt(imax), TInt(-1), OpExpected(imax, -1), 0, },
                { TInt(imax), TInt(1), OpExpected(imax, 1), 0, },
                { TInt(imax), TInt(2), OpExpected(imax, 2), s_ovf, },
                { TInt(imax), TInt(3), OpExpected(imax, 3), s_ovf, },

                { TInt(imin), TInt(imax - 3), OpExpected(imin, imax - 3), s_ovf, },
                { TInt(imin), TInt(imax - 2), OpExpected(imin, imax - 2), s_ovf, },
                { TInt(imin), TInt(imax - 1), OpExpected(imin, imax - 1), s_ovf, },

                { TInt(imax), TInt(imin + 1), OpExpected(imax, imin + 1), s_ovf, },
                { TInt(imax), TInt(imin + 2), OpExpected(imax, imin + 2), s_ovf, },
                { TInt(imax), TInt(imin + 3), OpExpected(imax, imin + 3), s_ovf, },

                #undef OpExpected
            });

            DoTest(TestType::Div, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1, _v2)        TInt(TInt(_v1) / TInt(_v2))
                // v1, v2, result_expected, state_expected

                { nan, 1, nan, s_inv, },

                { TInt(1), TInt(2), OpExpected(1, 2), 0, },
                { TInt(1), TInt(imax), OpExpected(1, imax), 0, },
                { TInt(-1), TInt(imin), OpExpected(-1, imin), 0, },

                // what the ...?
                { TInt(imin), TInt(-3), OpExpected(imin, -3), 0, },
                { TInt(imin), TInt(-2), OpExpected(imin, -2), 0, },
                { TInt(imin), TInt(-1), TInt(imin) /*-imin = imin*/, s_ovf, },
                { TInt(imin), TInt(1), OpExpected(imin, 1), 0, },
                { TInt(imin), TInt(2), OpExpected(imin, 2), 0, },
                { TInt(imin), TInt(3), OpExpected(imin, 3), 0, },

                { TInt(imin), TInt(imax), OpExpected(imin, imax), 0, },
                { TInt(imax), TInt(imin), OpExpected(imax, imin), 0, },

                { TInt(imax), TInt(-3), OpExpected(imax, -3), 0, },
                { TInt(imax), TInt(-2), OpExpected(imax, -2), 0, },
                { TInt(imax), TInt(-1), OpExpected(imax, -1), 0, },
                { TInt(imax), TInt(1), OpExpected(imax, 1), 0, },
                { TInt(imax), TInt(2), OpExpected(imax, 2), 0, },
                { TInt(imax), TInt(3), OpExpected(imax, 3), 0, },

                { TInt(imin), TInt(imax - 3), OpExpected(imin, imax - 3), 0, },
                { TInt(imin), TInt(imax - 2), OpExpected(imin, imax - 2), 0, },
                { TInt(imin), TInt(imax - 1), OpExpected(imin, imax - 1), 0, },

                { TInt(imax), TInt(imin + 1), OpExpected(imax, imin + 1), 0, },
                { TInt(imax), TInt(imin + 2), OpExpected(imax, imin + 2), 0, },
                { TInt(imax), TInt(imin + 3), OpExpected(imax, imin + 3), 0, },

                { TInt(imin), TInt(0), nan, s_inv | s_div, },
                { TInt(imax), TInt(0), nan, s_inv | s_div, },

                #undef OpExpected
            });

            DoTest(TestType::Rem, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1, _v2)        TInt(   \
                    ( ((_v1) ^ (_v2)) & signbit ? -1 : 1 )  \
                        * std::abs(TInt(_v1) % TInt(_v2))   \
                )
                // v1, v2, result_expected, state_expected

                { nan, 1, nan, s_inv, },

                { TInt(1), TInt(2), OpExpected(1, 2), 0, },
                { TInt(1), TInt(imax), OpExpected(1, imax), 0, },
                { TInt(-1), TInt(imin), OpExpected(-1, imin), 0, },

                { TInt(imin), TInt(-3), OpExpected(imin, -3), 0, },
                { TInt(imin), TInt(-2), OpExpected(imin, -2), 0, },
                { TInt(imin), TInt(-1), 0 /*-imin = imin*/, 0, },
                { TInt(imin), TInt(1), OpExpected(imin, 1), 0, },
                { TInt(imin), TInt(2), OpExpected(imin, 2), 0, },
                { TInt(imin), TInt(3), OpExpected(imin, 3), 0, },

                { TInt(imin), TInt(imax), OpExpected(imin, imax), 0, },
                { TInt(imax), TInt(imin), OpExpected(imax, imin), 0, },

                { TInt(imax), TInt(-3), OpExpected(imax, -3), 0, },
                { TInt(imax), TInt(-2), OpExpected(imax, -2), 0, },
                { TInt(imax), TInt(-1), OpExpected(imax, -1), 0, },
                { TInt(imax), TInt(1), OpExpected(imax, 1), 0, },
                { TInt(imax), TInt(2), OpExpected(imax, 2), 0, },
                { TInt(imax), TInt(3), OpExpected(imax, 3), 0, },

                { TInt(imin), TInt(imax - 3), OpExpected(imin, imax - 3), 0, },
                { TInt(imin), TInt(imax - 2), OpExpected(imin, imax - 2), 0, },
                { TInt(imin), TInt(imax - 1), OpExpected(imin, imax - 1), 0, },

                { TInt(imax), TInt(imin + 1), OpExpected(imax, imin + 1), 0, },
                { TInt(imax), TInt(imin + 2), OpExpected(imax, imin + 2), 0, },
                { TInt(imax), TInt(imin + 3), OpExpected(imax, imin + 3), 0, },

                { TInt(imin), TInt(0), nan, s_inv | s_div, },
                { TInt(imax), TInt(0), nan, s_inv | s_div, },

                #undef OpExpected
            });

            DoTest(TestType::Shl, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1, _v2)        TInt(   \
                    (TUInt(_v2) >= bitwidth) ? 0 : (        \
                        (TUInt(_v1)) << TUInt(              \
                            (TUInt(_v2)) & (bitwidth - 1)   \
                        )                                   \
                    ))
                // v1, v2, result_expected, state_expected

                { nan, 1, nan, s_inv, },

                { TInt(1), TInt(2), OpExpected(1, 2), 0, },
                { TInt(1), TInt(imax), OpExpected(1, imax), s_ovf, },
                { TInt(-1), TInt(imin), nan, s_inv, },

                { TInt(imin), TInt(-3), nan, s_inv, },
                { TInt(imin), TInt(-2), nan, s_inv, },
                { TInt(imin), TInt(-1), nan, s_inv, },
                { TInt(imin), TInt(1), OpExpected(imin, 1), s_ovf, },
                { TInt(imin), TInt(2), OpExpected(imin, 2), s_ovf, },
                { TInt(imin), TInt(3), OpExpected(imin, 3), s_ovf, },

                { TInt(imin), TInt(imax), OpExpected(imin, imax), s_ovf, },
                { TInt(imax), TInt(imin), nan, s_inv, },

                { TInt(imax), TInt(-3), nan, s_inv, },
                { TInt(imax), TInt(-2), nan, s_inv, },
                { TInt(imax), TInt(-1), nan, s_inv, },
                { TInt(imax), TInt(1), OpExpected(imax, 1), s_ovf, },
                { TInt(imax), TInt(2), OpExpected(imax, 2), s_ovf, },
                { TInt(imax), TInt(3), OpExpected(imax, 3), s_ovf, },

                { TInt(imin), TInt(imax - 3), OpExpected(imin, imax - 3), s_ovf, },
                { TInt(imin), TInt(imax - 2), OpExpected(imin, imax - 2), s_ovf, },
                { TInt(imin), TInt(imax - 1), OpExpected(imin, imax - 1), s_ovf, },

                { TInt(imax), TInt(imin + 1), nan, s_inv, },
                { TInt(imax), TInt(imin + 2), nan, s_inv, },
                { TInt(imax), TInt(imin + 3), nan, s_inv, },

                { TInt(0), TInt(imax - 3), OpExpected(0, imax - 3), 0, },
                { TInt(0), TInt(imax - 2), OpExpected(0, imax - 2), 0, },
                { TInt(0), TInt(imax - 1), OpExpected(0, imax - 1), 0, },

                { TInt(0), TInt(imin + 1), nan, s_inv, },
                { TInt(0), TInt(imin + 2), nan, s_inv, },
                { TInt(0), TInt(imin + 3), nan, s_inv, },

                #undef OpExpected
            });

            DoTest(TestType::Shr, std::vector<Testcase<TInt>>
            {
                // defined if _v2 >= 0
                #define OpExpected(_v1, _v2)        TInt(Shr_Signed_Helper(TUInt(_v1), TUInt(_v2)))
                // v1, v2, result_expected, state_expected

                { nan, 1, nan, s_inv, },

                { TInt(1), TInt(2), OpExpected(1, 2), 0, },
                { TInt(1), TInt(imax), OpExpected(1, imax), 0, },
                { TInt(-1), TInt(imin), nan, s_inv, },

                { TInt(imin), TInt(-3), nan, s_inv, },
                { TInt(imin), TInt(-2), nan, s_inv, },
                { TInt(imin), TInt(-1), nan, s_inv, },
                { TInt(imin), TInt(1), OpExpected(imin, 1), 0, },
                { TInt(imin), TInt(2), OpExpected(imin, 2), 0, },
                { TInt(imin), TInt(3), OpExpected(imin, 3), 0, },

                { TInt(imin), TInt(imax), OpExpected(imin, imax), 0, },
                { TInt(imax), TInt(imin), nan, s_inv, },

                { TInt(imax), TInt(-3), nan, s_inv, },
                { TInt(imax), TInt(-2), nan, s_inv, },
                { TInt(imax), TInt(-1), nan, s_inv, },
                { TInt(imax), TInt(1), OpExpected(imax, 1), 0, },
                { TInt(imax), TInt(2), OpExpected(imax, 2), 0, },
                { TInt(imax), TInt(3), OpExpected(imax, 3), 0, },

                { TInt(imin), TInt(imax - 3), OpExpected(imin, imax - 3), 0, },
                { TInt(imin), TInt(imax - 2), OpExpected(imin, imax - 2), 0, },
                { TInt(imin), TInt(imax - 1), OpExpected(imin, imax - 1), 0, },

                { TInt(imax), TInt(imin + 1), nan, s_inv, },
                { TInt(imax), TInt(imin + 2), nan, s_inv, },
                { TInt(imax), TInt(imin + 3), nan, s_inv, },

                #undef OpExpected
            });

            DoTest(TestType::And, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1, _v2)        TInt(TUInt(_v1) & TUInt(_v2))
                // v1, v2, result_expected, state_expected

                { nan, 1, nan, s_inv, },

                { TInt(1), TInt(2), OpExpected(1, 2), 0, },
                { TInt(1), TInt(imax), OpExpected(1, imax), 0, },
                { TInt(-1), TInt(imin), OpExpected(-1, imin), 0, },

                { TInt(imin), TInt(-3), OpExpected(imin, -3), 0, },
                { TInt(imin), TInt(-2), OpExpected(imin, -2), 0, },
                { TInt(imin), TInt(-1), OpExpected(imin, -1), 0, },
                { TInt(imin), TInt(1), OpExpected(imin, 1), 0, },
                { TInt(imin), TInt(2), OpExpected(imin, 2), 0, },
                { TInt(imin), TInt(3), OpExpected(imin, 3), 0, },

                { TInt(imin), TInt(imax), OpExpected(imin, imax), 0, },
                { TInt(imax), TInt(imin), OpExpected(imax, imin), 0, },

                { TInt(imax), TInt(-3), OpExpected(imax, -3), 0, },
                { TInt(imax), TInt(-2), OpExpected(imax, -2), 0, },
                { TInt(imax), TInt(-1), OpExpected(imax, -1), 0, },
                { TInt(imax), TInt(1), OpExpected(imax, 1), 0, },
                { TInt(imax), TInt(2), OpExpected(imax, 2), 0, },
                { TInt(imax), TInt(3), OpExpected(imax, 3), 0, },

                { TInt(imin), TInt(imax - 3), OpExpected(imin, imax - 3), 0, },
                { TInt(imin), TInt(imax - 2), OpExpected(imin, imax - 2), 0, },
                { TInt(imin), TInt(imax - 1), OpExpected(imin, imax - 1), 0, },

                { TInt(imax), TInt(imin + 1), OpExpected(imax, imin + 1), 0, },
                { TInt(imax), TInt(imin + 2), OpExpected(imax, imin + 2), 0, },
                { TInt(imax), TInt(imin + 3), OpExpected(imax, imin + 3), 0, },

                #undef OpExpected
            });

            DoTest(TestType::Or, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1, _v2)        TInt(TUInt(_v1) | TUInt(_v2))
                // v1, v2, result_expected, state_expected

                { nan, 1, nan, s_inv, },

                { TInt(1), TInt(2), OpExpected(1, 2), 0, },
                { TInt(1), TInt(imax), OpExpected(1, imax), 0, },
                { TInt(-1), TInt(imin), OpExpected(-1, imin), 0, },

                { TInt(imin), TInt(-3), OpExpected(imin, -3), 0, },
                { TInt(imin), TInt(-2), OpExpected(imin, -2), 0, },
                { TInt(imin), TInt(-1), OpExpected(imin, -1), 0, },
                { TInt(imin), TInt(1), OpExpected(imin, 1), 0, },
                { TInt(imin), TInt(2), OpExpected(imin, 2), 0, },
                { TInt(imin), TInt(3), OpExpected(imin, 3), 0, },

                { TInt(imin), TInt(imax), OpExpected(imin, imax), 0, },
                { TInt(imax), TInt(imin), OpExpected(imax, imin), 0, },

                { TInt(imax), TInt(-3), OpExpected(imax, -3), 0, },
                { TInt(imax), TInt(-2), OpExpected(imax, -2), 0, },
                { TInt(imax), TInt(-1), OpExpected(imax, -1), 0, },
                { TInt(imax), TInt(1), OpExpected(imax, 1), 0, },
                { TInt(imax), TInt(2), OpExpected(imax, 2), 0, },
                { TInt(imax), TInt(3), OpExpected(imax, 3), 0, },

                { TInt(imin), TInt(imax - 3), OpExpected(imin, imax - 3), 0, },
                { TInt(imin), TInt(imax - 2), OpExpected(imin, imax - 2), 0, },
                { TInt(imin), TInt(imax - 1), OpExpected(imin, imax - 1), 0, },

                { TInt(imax), TInt(imin + 1), OpExpected(imax, imin + 1), 0, },
                { TInt(imax), TInt(imin + 2), OpExpected(imax, imin + 2), 0, },
                { TInt(imax), TInt(imin + 3), OpExpected(imax, imin + 3), 0, },

                #undef OpExpected
            });

            DoTest(TestType::Xor, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1, _v2)        TInt(TUInt(_v1) ^ TUInt(_v2))
                // v1, v2, result_expected, state_expected

                { nan, 1, nan, s_inv, },

                { TInt(1), TInt(2), OpExpected(1, 2), 0, },
                { TInt(1), TInt(imax), OpExpected(1, imax), 0, },
                { TInt(-1), TInt(imin), OpExpected(-1, imin), 0, },

                { TInt(imin), TInt(-3), OpExpected(imin, -3), 0, },
                { TInt(imin), TInt(-2), OpExpected(imin, -2), 0, },
                { TInt(imin), TInt(-1), OpExpected(imin, -1), 0, },
                { TInt(imin), TInt(1), OpExpected(imin, 1), 0, },
                { TInt(imin), TInt(2), OpExpected(imin, 2), 0, },
                { TInt(imin), TInt(3), OpExpected(imin, 3), 0, },

                { TInt(imin), TInt(imax), OpExpected(imin, imax), 0, },
                { TInt(imax), TInt(imin), OpExpected(imax, imin), 0, },

                { TInt(imax), TInt(-3), OpExpected(imax, -3), 0, },
                { TInt(imax), TInt(-2), OpExpected(imax, -2), 0, },
                { TInt(imax), TInt(-1), OpExpected(imax, -1), 0, },
                { TInt(imax), TInt(1), OpExpected(imax, 1), 0, },
                { TInt(imax), TInt(2), OpExpected(imax, 2), 0, },
                { TInt(imax), TInt(3), OpExpected(imax, 3), 0, },

                { TInt(imin), TInt(imax - 3), OpExpected(imin, imax - 3), 0, },
                { TInt(imin), TInt(imax - 2), OpExpected(imin, imax - 2), 0, },
                { TInt(imin), TInt(imax - 1), OpExpected(imin, imax - 1), 0, },

                { TInt(imax), TInt(imin + 1), OpExpected(imax, imin + 1), 0, },
                { TInt(imax), TInt(imin + 2), OpExpected(imax, imin + 2), 0, },
                { TInt(imax), TInt(imin + 3), OpExpected(imax, imin + 3), 0, },

                #undef OpExpected
            });

            DoTest(TestType::Neg, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1)         TInt(-TUInt(_v1))
                // v1, v2, result_expected, state_expected

                { nan, 0, nan, s_inv, },

                { TInt(1), 0, OpExpected(1), 0, },
                { TInt(-1), 0, OpExpected(-1), 0, },

                { TInt(imin - 3), 0, OpExpected(imin - 3), 0, },
                { TInt(imin - 2), 0, OpExpected(imin - 2), 0, },
                { TInt(imin - 1), 0, OpExpected(imin - 1), 0, },
                { TInt(imin), 0, OpExpected(imin), s_ovf, },
                { TInt(imin + 1), 0, OpExpected(imin + 1), 0, },
                { TInt(imin + 2), 0, OpExpected(imin + 2), 0, },
                { TInt(imin + 3), 0, OpExpected(imin + 3), 0, },

                { TInt(imax - 3), 0, OpExpected(imax - 3), 0, },
                { TInt(imax - 2), 0, OpExpected(imax - 2), 0, },
                { TInt(imax - 1), 0, OpExpected(imax - 1), 0, },
                { TInt(imax), 0, OpExpected(imax), 0, },
                { TInt(imax + 1), 0, OpExpected(imax + 1), s_ovf, },
                { TInt(imax + 2), 0, OpExpected(imax + 2), 0, },
                { TInt(imax + 3), 0, OpExpected(imax + 3), 0, },

                #undef OpExpected
            });

            DoTest(TestType::Not, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1)         TInt(~TUInt(_v1))
                // v1, v2, result_expected, state_expected

                { nan, 0, nan, s_inv, },

                { TInt(1), 0, OpExpected(1), 0, },
                { TInt(-1), 0, OpExpected(-1), 0, },

                { TInt(imin - 3), 0, OpExpected(imin - 3), 0, },
                { TInt(imin - 2), 0, OpExpected(imin - 2), 0, },
                { TInt(imin - 1), 0, OpExpected(imin - 1), 0, },
                { TInt(imin), 0, OpExpected(imin), 0, },
                { TInt(imin + 1), 0, OpExpected(imin + 1), 0, },
                { TInt(imin + 2), 0, OpExpected(imin + 2), 0, },
                { TInt(imin + 3), 0, OpExpected(imin + 3), 0, },

                { TInt(imax - 3), 0, OpExpected(imax - 3), 0, },
                { TInt(imax - 2), 0, OpExpected(imax - 2), 0, },
                { TInt(imax - 1), 0, OpExpected(imax - 1), 0, },
                { TInt(imax), 0, OpExpected(imax), 0, },
                { TInt(imax + 1), 0, OpExpected(imax + 1), 0, },
                { TInt(imax + 2), 0, OpExpected(imax + 2), 0, },
                { TInt(imax + 3), 0, OpExpected(imax + 3), 0, },

                #undef OpExpected
            });

            DoTest(TestType::Inc, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1)         TInt(TUInt(_v1) + 1)
                // v1, v2, result_expected, state_expected

                { nan, 0, nan, s_inv, },

                { TInt(1), 0, OpExpected(1), 0, },
                { TInt(-1), 0, OpExpected(-1), 0, },

                { TInt(imin - 3), 0, OpExpected(imin - 3), 0, },
                { TInt(imin - 2), 0, OpExpected(imin - 2), 0, },
                { TInt(imin - 1), 0, OpExpected(imin - 1), s_ovf, },
                { TInt(imin), 0, OpExpected(imin), 0, },
                { TInt(imin + 1), 0, OpExpected(imin + 1), 0, },
                { TInt(imin + 2), 0, OpExpected(imin + 2), 0, },
                { TInt(imin + 3), 0, OpExpected(imin + 3), 0, },

                { TInt(imax - 3), 0, OpExpected(imax - 3), 0, },
                { TInt(imax - 2), 0, OpExpected(imax - 2), 0, },
                { TInt(imax - 1), 0, OpExpected(imax - 1), 0, },
                { TInt(imax), 0, OpExpected(imax), s_ovf, },
                { TInt(imax + 1), 0, OpExpected(imax + 1), 0, },
                { TInt(imax + 2), 0, OpExpected(imax + 2), 0, },
                { TInt(imax + 3), 0, OpExpected(imax + 3), 0, },

                #undef OpExpected
            });

            DoTest(TestType::Dec, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1)         TInt(TUInt(_v1) - 1)
                // v1, v2, result_expected, state_expected

                { nan, 0, nan, s_inv, },

                { TInt(1), 0, OpExpected(1), 0, },
                { TInt(-1), 0, OpExpected(-1), 0, },

                { TInt(imin - 3), 0, OpExpected(imin - 3), 0, },
                { TInt(imin - 2), 0, OpExpected(imin - 2), 0, },
                { TInt(imin - 1), 0, OpExpected(imin - 1), 0, },
                { TInt(imin), 0, OpExpected(imin), s_ovf, },
                { TInt(imin + 1), 0, OpExpected(imin + 1), 0, },
                { TInt(imin + 2), 0, OpExpected(imin + 2), 0, },
                { TInt(imin + 3), 0, OpExpected(imin + 3), 0, },

                { TInt(imax - 3), 0, OpExpected(imax - 3), 0, },
                { TInt(imax - 2), 0, OpExpected(imax - 2), 0, },
                { TInt(imax - 1), 0, OpExpected(imax - 1), 0, },
                { TInt(imax), 0, OpExpected(imax), 0, },
                { TInt(imax + 1), 0, OpExpected(imax + 1), s_ovf, },
                { TInt(imax + 2), 0, OpExpected(imax + 2), 0, },
                { TInt(imax + 3), 0, OpExpected(imax + 3), 0, },

                #undef OpExpected
            });
        }

        template <
            typename T,
            std::enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value, bool> = true>
            void DoMultipleTest()
        {
            using TInt = typename T;
            using TUInt = typename std::make_unsigned_t<TInt>;
            using TSInt = typename std::make_signed_t<TInt>;

            constexpr const TUInt imin = std::numeric_limits<TInt>::min();
            constexpr const TUInt imax = std::numeric_limits<TInt>::max();
            constexpr const TUInt bitwidth = sizeof(TInt) << 3;

            constexpr const uint8_t s_inv = Integer<TInt>::StateFlags::Invalid;
            constexpr const uint8_t s_ovf = Integer<TInt>::StateFlags::Overflow;
            constexpr const uint8_t s_div = Integer<TInt>::StateFlags::DivideByZero;

            Integer<TInt> nan;

            DoTest(TestType::Equ, std::vector<Testcase<TInt>>
            {
                // v1, v2, result_expected, state_expected

                { TInt(1), TInt(1), true, 0, },
                { TInt(1), TInt(2), false, 0, },
                { nan, TInt(1), false, 0, },

                { BaseInteger<TInt>(1, s_inv), BaseInteger<TInt>(1, s_inv), false, 0, },
                { BaseInteger<TInt>(1, s_inv), BaseInteger<TInt>(2, s_inv), false, 0, },
                { nan, nan, false, 0, },
            });

            DoTest(TestType::Neq, std::vector<Testcase<TInt>>
            {
                // v1, v2, result_expected, state_expected

                { TInt(1), TInt(1), false, 0, },
                { TInt(1), TInt(2), true, 0, },
                { nan, TInt(1), true, 0, },

                { BaseInteger<TInt>(1, s_inv), BaseInteger<TInt>(1, s_inv), true, 0, },
                { BaseInteger<TInt>(1, s_inv), BaseInteger<TInt>(2, s_inv), true, 0, },
                { nan, nan, true, 0, },
            });

            DoTest(TestType::Equ2, std::vector<Testcase<TInt>>
            {
                // v1, v2, result_expected, state_expected

                { TInt(1), TInt(1), true, 0, },
                { TInt(1), TInt(2), false, 0, },
                { nan, TInt(1), false, 0, },

                { BaseInteger<TInt>(1, s_inv), BaseInteger<TInt>(1, s_inv), true, 0, },
                { BaseInteger<TInt>(1, s_inv), BaseInteger<TInt>(2, s_inv), true, 0, },
                { nan, nan, true, 0, },
            });

            DoTest(TestType::Neq2, std::vector<Testcase<TInt>>
            {
                // v1, v2, result_expected, state_expected

                { TInt(1), TInt(1), false, 0, },
                { TInt(1), TInt(2), true, 0, },
                { nan, TInt(1), true, 0, },

                { BaseInteger<TInt>(1, s_inv), BaseInteger<TInt>(1, s_inv), false, 0, },
                { BaseInteger<TInt>(1, s_inv), BaseInteger<TInt>(2, s_inv), false, 0, },
                { nan, nan, false, 0, },
            });

            DoTest(TestType::Add, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1, _v2)        TUInt(TUInt(_v1) + TUInt(_v2))
                // v1, v2, result_expected, state_expected

                { nan, 1, nan, s_inv, },

                { TUInt(1), TUInt(2), OpExpected(1, 2), 0, },
                { TUInt(1), TUInt(imax), OpExpected(1, imax), s_ovf, },
                { TUInt(-1), TUInt(imin), OpExpected(-1, imin), 0, },

                { TUInt(imin), TUInt(-3), OpExpected(imin, -3), 0, },
                { TUInt(imin), TUInt(-2), OpExpected(imin, -2), 0, },
                { TUInt(imin), TUInt(-1), OpExpected(imin, -1), 0, },
                { TUInt(imin), TUInt(1), OpExpected(imin, 1), 0, },
                { TUInt(imin), TUInt(2), OpExpected(imin, 2), 0, },
                { TUInt(imin), TUInt(3), OpExpected(imin, 3), 0, },

                { TUInt(imin), TUInt(imax), OpExpected(imin, imax), 0, },
                { TUInt(imax), TUInt(imin), OpExpected(imax, imin), 0, },

                { TUInt(imax), TUInt(-3), OpExpected(imax, -3), s_ovf, },
                { TUInt(imax), TUInt(-2), OpExpected(imax, -2), s_ovf, },
                { TUInt(imax), TUInt(-1), OpExpected(imax, -1), s_ovf, },
                { TUInt(imax), TUInt(1), OpExpected(imax, 1), s_ovf, },
                { TUInt(imax), TUInt(2), OpExpected(imax, 2), s_ovf, },
                { TUInt(imax), TUInt(3), OpExpected(imax, 3), s_ovf, },

                { TUInt(imin), TUInt(imax - 3), OpExpected(imin, imax - 3), 0, },
                { TUInt(imin), TUInt(imax - 2), OpExpected(imin, imax - 2), 0, },
                { TUInt(imin), TUInt(imax - 1), OpExpected(imin, imax - 1), 0, },

                { TUInt(imax), TUInt(imin + 1), OpExpected(imax, imin + 1), s_ovf, },
                { TUInt(imax), TUInt(imin + 2), OpExpected(imax, imin + 2), s_ovf, },
                { TUInt(imax), TUInt(imin + 3), OpExpected(imax, imin + 3), s_ovf, },

                #undef OpExpected
            });

            DoTest(TestType::Sub, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1, _v2)        TUInt(TUInt(_v1) - TUInt(_v2))
                // v1, v2, result_expected, state_expected

                { nan, 1, nan, s_inv, },

                { TUInt(1), TUInt(2), OpExpected(1, 2), s_ovf, },
                { TUInt(1), TUInt(imax), OpExpected(1, imax), s_ovf, },
                { TUInt(-1), TUInt(imin), OpExpected(-1, imin), 0, },

                { TUInt(imin), TUInt(-3), OpExpected(imin, -3), s_ovf, },
                { TUInt(imin), TUInt(-2), OpExpected(imin, -2), s_ovf, },
                { TUInt(imin), TUInt(-1), OpExpected(imin, -1), s_ovf, },
                { TUInt(imin), TUInt(1), OpExpected(imin, 1), s_ovf, },
                { TUInt(imin), TUInt(2), OpExpected(imin, 2), s_ovf, },
                { TUInt(imin), TUInt(3), OpExpected(imin, 3), s_ovf, },

                { TUInt(imin), TUInt(imax), OpExpected(imin, imax), s_ovf, },
                { TUInt(imax), TUInt(imin), OpExpected(imax, imin), 0, },

                { TUInt(imax), TUInt(-3), OpExpected(imax, -3), 0, },
                { TUInt(imax), TUInt(-2), OpExpected(imax, -2), 0, },
                { TUInt(imax), TUInt(-1), OpExpected(imax, -1), 0, },
                { TUInt(imax), TUInt(1), OpExpected(imax, 1), 0, },
                { TUInt(imax), TUInt(2), OpExpected(imax, 2), 0, },
                { TUInt(imax), TUInt(3), OpExpected(imax, 3), 0, },

                { TUInt(imin), TUInt(imax - 3), OpExpected(imin, imax - 3), s_ovf, },
                { TUInt(imin), TUInt(imax - 2), OpExpected(imin, imax - 2), s_ovf, },
                { TUInt(imin), TUInt(imax - 1), OpExpected(imin, imax - 1), s_ovf, },

                { TUInt(imax), TUInt(imin + 1), OpExpected(imax, imin + 1), 0, },
                { TUInt(imax), TUInt(imin + 2), OpExpected(imax, imin + 2), 0, },
                { TUInt(imax), TUInt(imin + 3), OpExpected(imax, imin + 3), 0, },

                #undef OpExpected
            });

            DoTest(TestType::Mul, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1, _v2)        TUInt(TUInt(_v1) * TUInt(_v2))
                // v1, v2, result_expected, state_expected

                { nan, 1, nan, s_inv, },

                { TUInt(1), TUInt(2), OpExpected(1, 2), 0, },
                { TUInt(1), TUInt(imax), OpExpected(1, imax), 0, },
                { TUInt(-1), TUInt(imin), OpExpected(-1, imin), 0, },

                { TUInt(imin), TUInt(-3), OpExpected(imin, -3), 0, },
                { TUInt(imin), TUInt(-2), OpExpected(imin, -2), 0, },
                { TUInt(imin), TUInt(-1), OpExpected(imin, -1), 0, },
                { TUInt(imin), TUInt(1), OpExpected(imin, 1), 0, },
                { TUInt(imin), TUInt(2), OpExpected(imin, 2), 0, },
                { TUInt(imin), TUInt(3), OpExpected(imin, 3), 0, },

                { TUInt(imin), TUInt(imax), OpExpected(imin, imax), 0, },
                { TUInt(imax), TUInt(imin), OpExpected(imax, imin), 0, },

                { TUInt(imax), TUInt(-3), OpExpected(imax, -3), s_ovf, },
                { TUInt(imax), TUInt(-2), OpExpected(imax, -2), s_ovf, },
                { TUInt(imax), TUInt(-1), OpExpected(imax, -1), s_ovf, },
                { TUInt(imax), TUInt(1), OpExpected(imax, 1), 0, },
                { TUInt(imax), TUInt(2), OpExpected(imax, 2), s_ovf, },
                { TUInt(imax), TUInt(3), OpExpected(imax, 3), s_ovf, },

                { TUInt(imin), TUInt(imax - 3), OpExpected(imin, imax - 3), 0, },
                { TUInt(imin), TUInt(imax - 2), OpExpected(imin, imax - 2), 0, },
                { TUInt(imin), TUInt(imax - 1), OpExpected(imin, imax - 1), 0, },

                { TUInt(imax), TUInt(imin + 1), OpExpected(imax, imin + 1), 0, },
                { TUInt(imax), TUInt(imin + 2), OpExpected(imax, imin + 2), s_ovf, },
                { TUInt(imax), TUInt(imin + 3), OpExpected(imax, imin + 3), s_ovf, },

                #undef OpExpected
            });

            DoTest(TestType::Div, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1, _v2)        TUInt(TUInt(_v1) / TUInt(_v2))
                // v1, v2, result_expected, state_expected

                { nan, 1, nan, s_inv, },

                { TUInt(1), TUInt(2), OpExpected(1, 2), 0, },
                { TUInt(1), TUInt(imax), OpExpected(1, imax), 0, },
                { TUInt(-1), TUInt(imin), nan, s_inv | s_div, },

                { TUInt(imin), TUInt(-3), OpExpected(imin, -3), 0, },
                { TUInt(imin), TUInt(-2), OpExpected(imin, -2), 0, },
                { TUInt(imin), TUInt(-1), OpExpected(imin, -1), 0, },
                { TUInt(imin), TUInt(1), OpExpected(imin, 1), 0, },
                { TUInt(imin), TUInt(2), OpExpected(imin, 2), 0, },
                { TUInt(imin), TUInt(3), OpExpected(imin, 3), 0, },

                { TUInt(imin), TUInt(imax), OpExpected(imin, imax), 0, },
                { TUInt(imax), TUInt(imin), nan, s_inv | s_div, },

                { TUInt(imax), TUInt(-3), OpExpected(imax, -3), 0, },
                { TUInt(imax), TUInt(-2), OpExpected(imax, -2), 0, },
                { TUInt(imax), TUInt(-1), OpExpected(imax, -1), 0, },
                { TUInt(imax), TUInt(1), OpExpected(imax, 1), 0, },
                { TUInt(imax), TUInt(2), OpExpected(imax, 2), 0, },
                { TUInt(imax), TUInt(3), OpExpected(imax, 3), 0, },

                { TUInt(imin), TUInt(imax - 3), OpExpected(imin, imax - 3), 0, },
                { TUInt(imin), TUInt(imax - 2), OpExpected(imin, imax - 2), 0, },
                { TUInt(imin), TUInt(imax - 1), OpExpected(imin, imax - 1), 0, },

                { TUInt(imax), TUInt(imin + 1), OpExpected(imax, imin + 1), 0, },
                { TUInt(imax), TUInt(imin + 2), OpExpected(imax, imin + 2), 0, },
                { TUInt(imax), TUInt(imin + 3), OpExpected(imax, imin + 3), 0, },

                { TUInt(imin), TUInt(0), nan, s_inv | s_div, },
                { TUInt(imax), TUInt(0), nan, s_inv | s_div, },

                #undef OpExpected
            });

            DoTest(TestType::Rem, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1, _v2)        TUInt(TUInt(_v1) % TUInt(_v2))
                // v1, v2, result_expected, state_expected

                { nan, 1, nan, s_inv, },

                { TUInt(1), TUInt(2), OpExpected(1, 2), 0, },
                { TUInt(1), TUInt(imax), OpExpected(1, imax), 0, },
                { TUInt(-1), TUInt(imin), nan, s_inv | s_div, },

                { TUInt(imin), TUInt(-3), OpExpected(imin, -3), 0, },
                { TUInt(imin), TUInt(-2), OpExpected(imin, -2), 0, },
                { TUInt(imin), TUInt(-1), OpExpected(imin, -1), 0, },
                { TUInt(imin), TUInt(1), OpExpected(imin, 1), 0, },
                { TUInt(imin), TUInt(2), OpExpected(imin, 2), 0, },
                { TUInt(imin), TUInt(3), OpExpected(imin, 3), 0, },

                { TUInt(imin), TUInt(imax), OpExpected(imin, imax), 0, },
                { TUInt(imax), TUInt(imin), nan, s_inv | s_div, },

                { TUInt(imax), TUInt(-3), OpExpected(imax, -3), 0, },
                { TUInt(imax), TUInt(-2), OpExpected(imax, -2), 0, },
                { TUInt(imax), TUInt(-1), OpExpected(imax, -1), 0, },
                { TUInt(imax), TUInt(1), OpExpected(imax, 1), 0, },
                { TUInt(imax), TUInt(2), OpExpected(imax, 2), 0, },
                { TUInt(imax), TUInt(3), OpExpected(imax, 3), 0, },

                { TUInt(imin), TUInt(imax - 3), OpExpected(imin, imax - 3), 0, },
                { TUInt(imin), TUInt(imax - 2), OpExpected(imin, imax - 2), 0, },
                { TUInt(imin), TUInt(imax - 1), OpExpected(imin, imax - 1), 0, },

                { TUInt(imax), TUInt(imin + 1), OpExpected(imax, imin + 1), 0, },
                { TUInt(imax), TUInt(imin + 2), OpExpected(imax, imin + 2), 0, },
                { TUInt(imax), TUInt(imin + 3), OpExpected(imax, imin + 3), 0, },

                { TUInt(imin), TUInt(0), nan, s_inv | s_div, },
                { TUInt(imax), TUInt(0), nan, s_inv | s_div, },

                #undef OpExpected
            });

            DoTest(TestType::Shl, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1, _v2)        TUInt(  \
                    (TUInt(_v2) >= bitwidth) ? 0 : (        \
                        (TUInt(_v1)) << TUInt(              \
                            (TUInt(_v2)) & (bitwidth - 1)   \
                        )                                   \
                    ))
                // v1, v2, result_expected, state_expected

                { nan, 1, nan, s_inv, },

                { TUInt(1), TUInt(2), OpExpected(1, 2), 0, },
                { TUInt(1), TUInt(imax), OpExpected(1, imax), s_ovf, },
                { TUInt(-1), TUInt(imin), OpExpected(-1, imin), 0, },

                { TUInt(imin), TUInt(-3), OpExpected(imin, -3), 0, },
                { TUInt(imin), TUInt(-2), OpExpected(imin, -2), 0, },
                { TUInt(imin), TUInt(-1), OpExpected(imin, -1), 0, },
                { TUInt(imin), TUInt(1), OpExpected(imin, 1), 0, },
                { TUInt(imin), TUInt(2), OpExpected(imin, 2), 0, },
                { TUInt(imin), TUInt(3), OpExpected(imin, 3), 0, },

                { TUInt(imin), TUInt(imax), OpExpected(imin, imax), 0, },
                { TUInt(imax), TUInt(imin), OpExpected(imax, imin), 0, },

                { TUInt(imax), TUInt(-3), OpExpected(imax, -3), s_ovf, },
                { TUInt(imax), TUInt(-2), OpExpected(imax, -2), s_ovf, },
                { TUInt(imax), TUInt(-1), OpExpected(imax, -1), s_ovf, },
                { TUInt(imax), TUInt(1), OpExpected(imax, 1), s_ovf, },
                { TUInt(imax), TUInt(2), OpExpected(imax, 2), s_ovf, },
                { TUInt(imax), TUInt(3), OpExpected(imax, 3), s_ovf, },

                { TUInt(imin), TUInt(imax - 3), OpExpected(imin, imax - 3), 0, },
                { TUInt(imin), TUInt(imax - 2), OpExpected(imin, imax - 2), 0, },
                { TUInt(imin), TUInt(imax - 1), OpExpected(imin, imax - 1), 0, },

                { TUInt(imax), TUInt(imin + 1), OpExpected(imax, imin + 1), s_ovf, },
                { TUInt(imax), TUInt(imin + 2), OpExpected(imax, imin + 2), s_ovf, },
                { TUInt(imax), TUInt(imin + 3), OpExpected(imax, imin + 3), s_ovf, },

                { TInt(0), TInt(imax - 3), OpExpected(0, imax - 3), 0, },
                { TInt(0), TInt(imax - 2), OpExpected(0, imax - 2), 0, },
                { TInt(0), TInt(imax - 1), OpExpected(0, imax - 1), 0, },

                { TInt(0), TInt(imin + 1), OpExpected(0, imin + 1), 0, },
                { TInt(0), TInt(imin + 2), OpExpected(0, imin + 2), 0, },
                { TInt(0), TInt(imin + 3), OpExpected(0, imin + 3), 0, },

                #undef OpExpected
            });

            DoTest(TestType::Shr, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1, _v2)        TUInt(  \
                    (TUInt(_v2) >= bitwidth) ? 0 : (        \
                        (TUInt(_v1)) >> TUInt(              \
                            (TUInt(_v2)) & (bitwidth - 1)   \
                        )                                   \
                    ))
                // v1, v2, result_expected, state_expected

                { nan, 1, nan, s_inv, },

                { TUInt(1), TUInt(2), OpExpected(1, 2), 0, },
                { TUInt(1), TUInt(imax), OpExpected(1, imax), 0, },
                { TUInt(-1), TUInt(imin), OpExpected(-1, imin), 0, },

                { TUInt(imin), TUInt(-3), OpExpected(imin, -3), 0, },
                { TUInt(imin), TUInt(-2), OpExpected(imin, -2), 0, },
                { TUInt(imin), TUInt(-1), OpExpected(imin, -1), 0, },
                { TUInt(imin), TUInt(1), OpExpected(imin, 1), 0, },
                { TUInt(imin), TUInt(2), OpExpected(imin, 2), 0, },
                { TUInt(imin), TUInt(3), OpExpected(imin, 3), 0, },

                { TUInt(imin), TUInt(imax), OpExpected(imin, imax), 0, },
                { TUInt(imax), TUInt(imin), OpExpected(imax, imin), 0, },

                { TUInt(imax), TUInt(-3), OpExpected(imax, -3), 0, },
                { TUInt(imax), TUInt(-2), OpExpected(imax, -2), 0, },
                { TUInt(imax), TUInt(-1), OpExpected(imax, -1), 0, },
                { TUInt(imax), TUInt(1), OpExpected(imax, 1), 0, },
                { TUInt(imax), TUInt(2), OpExpected(imax, 2), 0, },
                { TUInt(imax), TUInt(3), OpExpected(imax, 3), 0, },

                { TUInt(imin), TUInt(imax - 3), OpExpected(imin, imax - 3), 0, },
                { TUInt(imin), TUInt(imax - 2), OpExpected(imin, imax - 2), 0, },
                { TUInt(imin), TUInt(imax - 1), OpExpected(imin, imax - 1), 0, },

                { TUInt(imax), TUInt(imin + 1), OpExpected(imax, imin + 1), 0, },
                { TUInt(imax), TUInt(imin + 2), OpExpected(imax, imin + 2), 0, },
                { TUInt(imax), TUInt(imin + 3), OpExpected(imax, imin + 3), 0, },

                #undef OpExpected
            });

            DoTest(TestType::And, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1, _v2)        TUInt(TUInt(_v1) & TUInt(_v2))
                // v1, v2, result_expected, state_expected

                { nan, 1, nan, s_inv, },

                { TUInt(1), TUInt(2), OpExpected(1, 2), 0, },
                { TUInt(1), TUInt(imax), OpExpected(1, imax), 0, },
                { TUInt(-1), TUInt(imin), OpExpected(-1, imin), 0, },

                { TUInt(imin), TUInt(-3), OpExpected(imin, -3), 0, },
                { TUInt(imin), TUInt(-2), OpExpected(imin, -2), 0, },
                { TUInt(imin), TUInt(-1), OpExpected(imin, -1), 0, },
                { TUInt(imin), TUInt(1), OpExpected(imin, 1), 0, },
                { TUInt(imin), TUInt(2), OpExpected(imin, 2), 0, },
                { TUInt(imin), TUInt(3), OpExpected(imin, 3), 0, },

                { TUInt(imin), TUInt(imax), OpExpected(imin, imax), 0, },
                { TUInt(imax), TUInt(imin), OpExpected(imax, imin), 0, },

                { TUInt(imax), TUInt(-3), OpExpected(imax, -3), 0, },
                { TUInt(imax), TUInt(-2), OpExpected(imax, -2), 0, },
                { TUInt(imax), TUInt(-1), OpExpected(imax, -1), 0, },
                { TUInt(imax), TUInt(1), OpExpected(imax, 1), 0, },
                { TUInt(imax), TUInt(2), OpExpected(imax, 2), 0, },
                { TUInt(imax), TUInt(3), OpExpected(imax, 3), 0, },

                { TUInt(imin), TUInt(imax - 3), OpExpected(imin, imax - 3), 0, },
                { TUInt(imin), TUInt(imax - 2), OpExpected(imin, imax - 2), 0, },
                { TUInt(imin), TUInt(imax - 1), OpExpected(imin, imax - 1), 0, },

                { TUInt(imax), TUInt(imin + 1), OpExpected(imax, imin + 1), 0, },
                { TUInt(imax), TUInt(imin + 2), OpExpected(imax, imin + 2), 0, },
                { TUInt(imax), TUInt(imin + 3), OpExpected(imax, imin + 3), 0, },

                #undef OpExpected
            });

            DoTest(TestType::Or, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1, _v2)        TUInt(TUInt(_v1) | TUInt(_v2))
                // v1, v2, result_expected, state_expected

                { nan, 1, nan, s_inv, },

                { TUInt(1), TUInt(2), OpExpected(1, 2), 0, },
                { TUInt(1), TUInt(imax), OpExpected(1, imax), 0, },
                { TUInt(-1), TUInt(imin), OpExpected(-1, imin), 0, },

                { TUInt(imin), TUInt(-3), OpExpected(imin, -3), 0, },
                { TUInt(imin), TUInt(-2), OpExpected(imin, -2), 0, },
                { TUInt(imin), TUInt(-1), OpExpected(imin, -1), 0, },
                { TUInt(imin), TUInt(1), OpExpected(imin, 1), 0, },
                { TUInt(imin), TUInt(2), OpExpected(imin, 2), 0, },
                { TUInt(imin), TUInt(3), OpExpected(imin, 3), 0, },

                { TUInt(imin), TUInt(imax), OpExpected(imin, imax), 0, },
                { TUInt(imax), TUInt(imin), OpExpected(imax, imin), 0, },

                { TUInt(imax), TUInt(-3), OpExpected(imax, -3), 0, },
                { TUInt(imax), TUInt(-2), OpExpected(imax, -2), 0, },
                { TUInt(imax), TUInt(-1), OpExpected(imax, -1), 0, },
                { TUInt(imax), TUInt(1), OpExpected(imax, 1), 0, },
                { TUInt(imax), TUInt(2), OpExpected(imax, 2), 0, },
                { TUInt(imax), TUInt(3), OpExpected(imax, 3), 0, },

                { TUInt(imin), TUInt(imax - 3), OpExpected(imin, imax - 3), 0, },
                { TUInt(imin), TUInt(imax - 2), OpExpected(imin, imax - 2), 0, },
                { TUInt(imin), TUInt(imax - 1), OpExpected(imin, imax - 1), 0, },

                { TUInt(imax), TUInt(imin + 1), OpExpected(imax, imin + 1), 0, },
                { TUInt(imax), TUInt(imin + 2), OpExpected(imax, imin + 2), 0, },
                { TUInt(imax), TUInt(imin + 3), OpExpected(imax, imin + 3), 0, },

                #undef OpExpected
            });

            DoTest(TestType::Xor, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1, _v2)        TUInt(TUInt(_v1) ^ TUInt(_v2))
                // v1, v2, result_expected, state_expected

                { nan, 1, nan, s_inv, },

                { TUInt(1), TUInt(2), OpExpected(1, 2), 0, },
                { TUInt(1), TUInt(imax), OpExpected(1, imax), 0, },
                { TUInt(-1), TUInt(imin), OpExpected(-1, imin), 0, },

                { TUInt(imin), TUInt(-3), OpExpected(imin, -3), 0, },
                { TUInt(imin), TUInt(-2), OpExpected(imin, -2), 0, },
                { TUInt(imin), TUInt(-1), OpExpected(imin, -1), 0, },
                { TUInt(imin), TUInt(1), OpExpected(imin, 1), 0, },
                { TUInt(imin), TUInt(2), OpExpected(imin, 2), 0, },
                { TUInt(imin), TUInt(3), OpExpected(imin, 3), 0, },

                { TUInt(imin), TUInt(imax), OpExpected(imin, imax), 0, },
                { TUInt(imax), TUInt(imin), OpExpected(imax, imin), 0, },

                { TUInt(imax), TUInt(-3), OpExpected(imax, -3), 0, },
                { TUInt(imax), TUInt(-2), OpExpected(imax, -2), 0, },
                { TUInt(imax), TUInt(-1), OpExpected(imax, -1), 0, },
                { TUInt(imax), TUInt(1), OpExpected(imax, 1), 0, },
                { TUInt(imax), TUInt(2), OpExpected(imax, 2), 0, },
                { TUInt(imax), TUInt(3), OpExpected(imax, 3), 0, },

                { TUInt(imin), TUInt(imax - 3), OpExpected(imin, imax - 3), 0, },
                { TUInt(imin), TUInt(imax - 2), OpExpected(imin, imax - 2), 0, },
                { TUInt(imin), TUInt(imax - 1), OpExpected(imin, imax - 1), 0, },

                { TUInt(imax), TUInt(imin + 1), OpExpected(imax, imin + 1), 0, },
                { TUInt(imax), TUInt(imin + 2), OpExpected(imax, imin + 2), 0, },
                { TUInt(imax), TUInt(imin + 3), OpExpected(imax, imin + 3), 0, },

                #undef OpExpected
            });

            DoTest(TestType::Neg, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1)         TUInt(-TUInt(_v1))
                // v1, v2, result_expected, state_expected

                { nan, 0, nan, s_inv, },

                { TUInt(1), 0, OpExpected(1), 0, },
                { TUInt(-1), 0, OpExpected(-1), 0, },

                { TUInt(imin - 3), 0, OpExpected(imin - 3), 0, },
                { TUInt(imin - 2), 0, OpExpected(imin - 2), 0, },
                { TUInt(imin - 1), 0, OpExpected(imin - 1), 0, },
                { TUInt(imin), 0, OpExpected(imin), 0, },
                { TUInt(imin + 1), 0, OpExpected(imin + 1), 0, },
                { TUInt(imin + 2), 0, OpExpected(imin + 2), 0, },
                { TUInt(imin + 3), 0, OpExpected(imin + 3), 0, },

                { TUInt(imax - 3), 0, OpExpected(imax - 3), 0, },
                { TUInt(imax - 2), 0, OpExpected(imax - 2), 0, },
                { TUInt(imax - 1), 0, OpExpected(imax - 1), 0, },
                { TUInt(imax), 0, OpExpected(imax), 0, },
                { TUInt(imax + 1), 0, OpExpected(imax + 1), 0, },
                { TUInt(imax + 2), 0, OpExpected(imax + 2), 0, },
                { TUInt(imax + 3), 0, OpExpected(imax + 3), 0, },

                #undef OpExpected
            });

            DoTest(TestType::Not, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1)         TUInt(~TUInt(_v1))
                // v1, v2, result_expected, state_expected

                { nan, 0, nan, s_inv, },

                { TUInt(1), 0, OpExpected(1), 0, },
                { TUInt(-1), 0, OpExpected(-1), 0, },

                { TUInt(imin - 3), 0, OpExpected(imin - 3), 0, },
                { TUInt(imin - 2), 0, OpExpected(imin - 2), 0, },
                { TUInt(imin - 1), 0, OpExpected(imin - 1), 0, },
                { TUInt(imin), 0, OpExpected(imin), 0, },
                { TUInt(imin + 1), 0, OpExpected(imin + 1), 0, },
                { TUInt(imin + 2), 0, OpExpected(imin + 2), 0, },
                { TUInt(imin + 3), 0, OpExpected(imin + 3), 0, },

                { TUInt(imax - 3), 0, OpExpected(imax - 3), 0, },
                { TUInt(imax - 2), 0, OpExpected(imax - 2), 0, },
                { TUInt(imax - 1), 0, OpExpected(imax - 1), 0, },
                { TUInt(imax), 0, OpExpected(imax), 0, },
                { TUInt(imax + 1), 0, OpExpected(imax + 1), 0, },
                { TUInt(imax + 2), 0, OpExpected(imax + 2), 0, },
                { TUInt(imax + 3), 0, OpExpected(imax + 3), 0, },

                #undef OpExpected
            });

            DoTest(TestType::Inc, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1)         TUInt(TUInt(_v1) + 1)
                // v1, v2, result_expected, state_expected

                { nan, 0, nan, s_inv, },

                { TUInt(1), 0, OpExpected(1), 0, },
                { TUInt(-1), 0, OpExpected(-1), s_ovf, },

                { TUInt(imin - 3), 0, OpExpected(imin - 3), 0, },
                { TUInt(imin - 2), 0, OpExpected(imin - 2), 0, },
                { TUInt(imin - 1), 0, OpExpected(imin - 1), s_ovf, },
                { TUInt(imin), 0, OpExpected(imin), 0, },
                { TUInt(imin + 1), 0, OpExpected(imin + 1), 0, },
                { TUInt(imin + 2), 0, OpExpected(imin + 2), 0, },
                { TUInt(imin + 3), 0, OpExpected(imin + 3), 0, },

                { TUInt(imax - 3), 0, OpExpected(imax - 3), 0, },
                { TUInt(imax - 2), 0, OpExpected(imax - 2), 0, },
                { TUInt(imax - 1), 0, OpExpected(imax - 1), 0, },
                { TUInt(imax), 0, OpExpected(imax), s_ovf, },
                { TUInt(imax + 1), 0, OpExpected(imax + 1), 0, },
                { TUInt(imax + 2), 0, OpExpected(imax + 2), 0, },
                { TUInt(imax + 3), 0, OpExpected(imax + 3), 0, },

                #undef OpExpected
            });

            DoTest(TestType::Dec, std::vector<Testcase<TInt>>
            {
                #define OpExpected(_v1)         TUInt(TUInt(_v1) - 1)
                // v1, v2, result_expected, state_expected

                { nan, 0, nan, s_inv, },

                { TUInt(1), 0, OpExpected(1), 0, },
                { TUInt(-1), 0, OpExpected(-1), 0, },

                { TUInt(imin - 3), 0, OpExpected(imin - 3), 0, },
                { TUInt(imin - 2), 0, OpExpected(imin - 2), 0, },
                { TUInt(imin - 1), 0, OpExpected(imin - 1), 0, },
                { TUInt(imin), 0, OpExpected(imin), s_ovf, },
                { TUInt(imin + 1), 0, OpExpected(imin + 1), 0, },
                { TUInt(imin + 2), 0, OpExpected(imin + 2), 0, },
                { TUInt(imin + 3), 0, OpExpected(imin + 3), 0, },

                { TUInt(imax - 3), 0, OpExpected(imax - 3), 0, },
                { TUInt(imax - 2), 0, OpExpected(imax - 2), 0, },
                { TUInt(imax - 1), 0, OpExpected(imax - 1), 0, },
                { TUInt(imax), 0, OpExpected(imax), 0, },
                { TUInt(imax + 1), 0, OpExpected(imax + 1), s_ovf, },
                { TUInt(imax + 2), 0, OpExpected(imax + 2), 0, },
                { TUInt(imax + 3), 0, OpExpected(imax + 3), 0, },

                #undef OpExpected
            });
        }
#include "pop_warnings.h"

        TEST_METHOD(Integer_MasterTest)
        {
            DoMultipleTest<int8_t>();
            DoMultipleTest<int16_t>();
            DoMultipleTest<int32_t>();
            DoMultipleTest<int64_t>();

            DoMultipleTest<uint8_t>();
            DoMultipleTest<uint16_t>();
            DoMultipleTest<uint32_t>();
            DoMultipleTest<uint64_t>();
        }

    };

    TEST_CLASS(MiscTest)
    {
    public:
        TEST_METHOD(Endianness_MasterTest)
        {
            uint32_t Value32 = 0x12345678;
            uint32_t Value32_2 = 0x78563412;
            LittleEndian<uint32_t> le32(Value32);
            BigEndian<uint32_t> be32(Value32);

            uint64_t Value64 = 0x123456789abcdef0;
            uint64_t Value64_2 = 0xf0debc9a78563412;
            LittleEndian<uint64_t> le64(Value64);
            BigEndian<uint64_t> be64(Value64);

            Logger::WriteMessage(L"testing endianness...");

            auto Endianness = Base::Endian();
            if (Endianness == Base::Endianness::Little)
            {
                Assert::AreEqual(le32.Get(), Value32);
                Assert::AreEqual(le32.GetRaw(), Value32);
                Assert::AreEqual(be32.Get(), Value32);
                Assert::AreEqual(be32.GetRaw(), Value32_2);

                Assert::AreEqual(le64.Get(), Value64);
                Assert::AreEqual(le64.GetRaw(), Value64);
                Assert::AreEqual(be64.Get(), Value64);
                Assert::AreEqual(be64.GetRaw(), Value64_2);
            }
            else if (Endianness == Base::Endianness::Big)
            {
                Assert::AreEqual(le32.Get(), Value32);
                Assert::AreEqual(le32.GetRaw(), Value32_2);
                Assert::AreEqual(be32.Get(), Value32);
                Assert::AreEqual(be32.GetRaw(), Value32);

                Assert::AreEqual(le64.Get(), Value64);
                Assert::AreEqual(le64.GetRaw(), Value64_2);
                Assert::AreEqual(be64.Get(), Value64);
                Assert::AreEqual(be64.GetRaw(), Value64);
            }
            else
            {
                Logger::WriteMessage(L"endianness unknown, skipping");
            }

        }

    private:
    };

}

