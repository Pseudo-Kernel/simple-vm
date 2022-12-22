#include "stdafx.h"
#include "unittest_helper.h"
#include "CppUnitTest.h"

#include <stdarg.h>
#include "../CoreStaticLib/svm/arch.h"
#include "../CoreStaticLib/svm/base.h"
#include "../CoreStaticLib/svm/vmbase.h"
#include "../CoreStaticLib/svm/vmmemory.h"
#include "../CoreStaticLib/svm/bc_interpreter.h"

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

