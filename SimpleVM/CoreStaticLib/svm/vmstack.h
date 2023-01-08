#pragma once

struct DataAreaRegister
{
    uint64_t Base;		// host pointer
    uint32_t Size;
    uint32_t Alignment;	// 1, 2, 4, 8, 16, 32, ...
    uint32_t Offset;
};

static_assert(
    std::is_standard_layout<DataAreaRegister>::value &&
    std::is_trivially_copyable<DataAreaRegister>::value,
    "type is not standard layout");


class VMStack : private DataAreaRegister
{
public:
    using BaseType = DataAreaRegister;
    using ByteType = unsigned char;
    using SizeType = decltype(BaseType::Size);

    constexpr VMStack() : BaseType() {}

    VMStack(const BaseType& State) : BaseType(State)
    {
        SanityCheck(State);
    }

    VMStack(uint64_t Base, size_t Size, uint32_t Alignment) :
        VMStack(Base, Size, Alignment, Size)
    {
    }

    VMStack(uint64_t Base, size_t Size, uint32_t Alignment, ptrdiff_t Offset)
    {
        BaseType State{};
        State.Base = Base;
        State.Size = Base::IntegerAssertCast<SizeType>(Size);
        State.Alignment = Alignment;
        State.Offset = Base::IntegerAssertCast<SizeType>(Offset);

        SanityCheck(State);

        *this = State;
    }

    ~VMStack() = default;

    template <
        typename T,
        std::enable_if_t<
            std::is_integral<T>::value && !std::is_signed<T>::value, bool> = true>
        bool Push(const T& Value) noexcept
    {
        // unsigned integer type
        constexpr const auto Size = sizeof(T);
        constexpr const auto MaxIntSize = sizeof(int64_t);

        static_assert(
            Size == 1 || Size == 2 || Size == 4 || Size == 8,
            "Unexpected size of T (only 1, 2, 4, 8 is allowed)");

        alignas(MaxIntSize) ByteType Temp[MaxIntSize]{};

        if (BaseType::Alignment <= Size)
        {
            auto Converted = Base::BitCast<Base::ToIntegralType<T>::type>(Value);
            Base::ToBytes(Converted, Temp);
            return Push(Temp, Size);
        }
        else
        {
            // NOTE: Push zero-extended value

            // T                       Value     Alignment                      Result
            // uint8_t                    80             4                   0000'0080
            // uint16_t                 8000             4                   0000'8000
            // uint32_t            8000'0000             4                   8000'0000
            // uint64_t  0000'0000'8000'0000             4         0000'0000'8000'0000
            // uint64_t  8000'0000'0000'0000             4         8000'0000'0000'0000
            // uint8_t                    80             8         0000'0000'0000'0080
            // uint16_t                 8000             8         0000'0000'0000'8000
            // uint32_t            8000'0000             8         0000'0000'8000'0000
            // uint8_t                    7f             4                   0000'007f
            // uint16_t                 7fff             4                   0000'7fff
            // uint32_t            7fff'ffff             4                   7fff'ffff
            // uint64_t  0000'0000'7fff'ffff             4         0000'0000'7fff'ffff
            // uint64_t  7fff'ffff'ffff'ffff             4         7fff'ffff'ffff'ffff
            // uint8_t                    7f             8         0000'0000'0000'007f
            // uint16_t                 7fff             8         0000'0000'0000'7fff
            // uint32_t            7fff'ffff             8         0000'0000'7fff'ffff

            if (BaseType::Alignment == sizeof(int32_t))
            {
                using ZeroExtendedType = uint32_t;
                auto Extended = Base::ZeroExtend<ZeroExtendedType>(Value);
                Base::ToBytes(Extended, Temp);
                return Push(Temp, sizeof(ZeroExtendedType));
            }
            else if (BaseType::Alignment == sizeof(int64_t))
            {
                using ZeroExtendedType = uint64_t;
                auto Extended = Base::ZeroExtend<ZeroExtendedType>(Value);
                Base::ToBytes(Extended, Temp);
                return Push(Temp, sizeof(ZeroExtendedType));
            }
        }

        DASSERT(false);
        return false;

#if 0
        // unsigned integer type
        constexpr const auto Size = sizeof(T);
        ByteType Temp[Size]{};

        auto Converted = Base::BitCast<Base::ToIntegralType<T>::type>(Value);
        Base::ToBytes(Converted, Temp);
        return Push(Temp, Size);
#endif
    }

    template <
        typename T,
        std::enable_if_t<
            std::is_integral<T>::value && std::is_signed<T>::value, bool> = true>
        bool Push(const T& Value) noexcept
    {
        // signed integer type
        constexpr const auto Size = sizeof(T);
        constexpr const auto MaxIntSize = sizeof(int64_t);

        static_assert(
            Size == 1 || Size == 2 || Size == 4 || Size == 8,
            "Unexpected size of T (only 1, 2, 4, 8 is allowed)");

        alignas(MaxIntSize) ByteType Temp[MaxIntSize]{};

        if (BaseType::Alignment <= Size)
        {
            auto Converted = Base::BitCast<Base::ToIntegralType<T>::type>(Value);
            Base::ToBytes(Converted, Temp);
            return Push(Temp, Size);
        }
        else
        {
            // NOTE: Push sign-extended value

            // T                       Value     Alignment                      Result
            // int8_t                     80             4                   ffff'ff80
            // int16_t                  8000             4                   ffff'8000
            // int32_t             8000'0000             4                   8000'0000
            // int64_t   0000'0000'8000'0000             4         0000'0000'8000'0000
            // int64_t   8000'0000'0000'0000             4         8000'0000'0000'0000
            // int8_t                     80             8         ffff'ffff'ffff'ff80
            // int16_t                  8000             8         ffff'ffff'ffff'8000
            // int32_t             8000'0000             8         ffff'ffff'8000'0000
            // int8_t                     7f             4                   0000'007f
            // int16_t                  7fff             4                   0000'7fff
            // int32_t             7fff'ffff             4                   7fff'ffff
            // int64_t   0000'0000'7fff'ffff             4         0000'0000'7fff'ffff
            // int64_t   7fff'ffff'ffff'ffff             4         7fff'ffff'ffff'ffff
            // int8_t                     7f             8         0000'0000'0000'007f
            // int16_t                  7fff             8         0000'0000'0000'7fff
            // int32_t             7fff'ffff             8         0000'0000'7fff'ffff

            if (BaseType::Alignment == sizeof(int32_t))
            {
                using SignExtendedType = int32_t;
                auto Extended = Base::SignExtend<SignExtendedType>(Value);
                Base::ToBytes(Extended, Temp);
                return Push(Temp, sizeof(SignExtendedType));
            }
            else if (BaseType::Alignment == sizeof(int64_t))
            {
                using SignExtendedType = int64_t;
                auto Extended = Base::SignExtend<SignExtendedType>(Value);
                Base::ToBytes(Extended, Temp);
                return Push(Temp, sizeof(SignExtendedType));
            }
        }

        DASSERT(false);
        return false;
    }

    template <
        typename T,
        std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
        bool Push(const T& Value) noexcept
    {
        using TSignedInt = typename std::make_signed_t<typename Base::ToIntegralType<T>::type>;
        return Push(Base::BitCast<TSignedInt>(Value)); // use safer cast
    }

    template <
        typename T,
        std::enable_if_t<
        std::is_trivially_copyable<T>::value &&
        !std::is_integral<T>::value &&
        !std::is_floating_point<T>::value, bool> = true>
        bool Push(const T& Value) noexcept
    {
        return Write(Value, true);
    }

    bool Push(const ByteType* Buffer, size_t Size) noexcept
    {
        return Write(Buffer, Size, true);
    }

    template <
        typename T,
        std::enable_if_t<std::is_trivially_copyable<T>::value, bool> = true>
        bool Pop(T* Value) noexcept
    {
        return Read(Value, true);
    }

    bool Pop(ByteType* Buffer, size_t Size) noexcept
    {
        return Read(Buffer, Size, true);
    }

    template <
        typename T,
        std::enable_if_t<std::is_trivially_copyable<T>::value, bool> = true>
        bool PeekFrom(T* Value, int OffsetFromCurrent) noexcept
    {
        return ReadFrom(Value, OffsetFromCurrent);
    }

    bool Peek(ByteType* Buffer, size_t Size) noexcept
    {
        return Read(Buffer, Size);
    }

    bool SetTopOffset(uint32_t TopOffset) noexcept
    {
        // check out of bounds
        if (!IsOffsetValid(TopOffset, true))
            return false;

        BaseType::Offset = TopOffset;
        return true;
    }

    uint64_t Top() const noexcept
    {
        return BaseType::Base + BaseType::Offset;
    }

    uint32_t TopOffset() const noexcept
    {
        return BaseType::Offset;
    }

    auto Alignment() const noexcept
    {
        return BaseType::Alignment;
    }

private:

    unsigned char* Pointer(uint32_t Offset) const noexcept
    {
        if (!IsOffsetValid(Offset, false))
            return nullptr;

        return reinterpret_cast<unsigned char*>(BaseType::Base + Offset);
    }

    bool IsOffsetValid(uint32_t Offset, bool StackTop) const noexcept
    {
        if (0 <= Offset && Offset < BaseType::Size)
        {
            return true;
        }

        if (StackTop && BaseType::Size == Offset)
        {
            return true;
        }

        return false;
    }

    bool ValidateForRead(int OffsetFromCurrent, size_t Size, uint32_t* BeforeRead, uint32_t* ReadAfter) noexcept
    {
        DASSERT(!(Top() & (Alignment() - 1)));

        SizeType SizeCasted = 0;
        if (!Base::IntegerTestCast<SizeType>(Size, SizeCasted))
            return false;

        const auto AlignmentMask = Alignment() - 1;
        const auto SizeAligned = (SizeCasted + AlignmentMask) & ~AlignmentMask;

        auto OffsetStart = TopOffset() + OffsetFromCurrent;
        auto OffsetEnd = TopOffset() + OffsetFromCurrent + SizeAligned;

        // check out of bounds
        if (!(OffsetStart < OffsetEnd) ||
            !IsOffsetValid(OffsetStart, true) ||
            !IsOffsetValid(OffsetEnd, true))
            return false;

        // Read:
        //   ReadAddr = Curr
        //   ReadFrom(ReadAddr)
        //   Curr = ReadAddr + Size

        if (BeforeRead)
            *BeforeRead = OffsetStart;

        if (ReadAfter)
            *ReadAfter = OffsetEnd;

        return true;
    }

    bool ValidateForWrite(int OffsetFromCurrent, size_t Size, uint32_t* BeforeWrite, uint32_t* WriteAfter) noexcept
    {
        DASSERT(!(Top() & (Alignment() - 1)));

        SizeType SizeCasted = 0;
        if (!Base::IntegerTestCast<SizeType>(Size, SizeCasted))
            return false;

        const auto AlignmentMask = Alignment() - 1;
        const auto SizeAligned = (SizeCasted + AlignmentMask) & ~AlignmentMask;

        auto OffsetStart = TopOffset() + OffsetFromCurrent - SizeAligned;
        auto OffsetEnd = TopOffset() + OffsetFromCurrent;

        // check out of bounds
        if (!(OffsetStart < OffsetEnd) ||
            !IsOffsetValid(OffsetStart, true) ||
            !IsOffsetValid(OffsetEnd, true))
            return false;

        // Write:
        //   WriteAddr = Curr - Size
        //   WriteTo(WriteAddr)
        //   Curr = WriteAddr

        if (BeforeWrite)
            *BeforeWrite = OffsetEnd;

        if (WriteAfter)
            *WriteAfter = OffsetStart;

        return true;
    }

    template <
        typename T,
        std::enable_if_t<std::is_trivially_copyable<T>::value, bool> = true>
        bool Read(T* Value, bool Discard = false) noexcept
    {
        uint32_t Target = 0, New = 0;

        if (!ValidateForRead(0, sizeof(T), &Target, &New))
            return false;

        if (Value)
            *Value = Base::FromBytes<T>(Pointer(Target));

        // update pointer if needed
        if (Discard)
            BaseType::Offset = New;

        return true;
    }

    template <
        typename T,
        std::enable_if_t<std::is_trivially_copyable<T>::value, bool> = true>
        bool ReadFrom(T* Value, int OffsetFromCurrent) noexcept
    {
        uint32_t Target = 0, New = 0;

        if (!ValidateForRead(OffsetFromCurrent, sizeof(T), &Target, &New))
            return false;

        if (Value)
            *Value = Base::FromBytes<T>(Pointer(Target));

        return true;
    }

    bool Read(ByteType* Buffer, size_t Size, bool Discard = false) noexcept
    {
        uint32_t Target = 0, New = 0;

        SizeType SizeCasted = 0;
        if (!Base::IntegerTestCast<SizeType>(Size, SizeCasted))
            return false;

        if (!ValidateForRead(0, SizeCasted, &Target, &New))
            return false;

        if (Buffer)
            std::memcpy(Buffer, Pointer(Target), SizeCasted);

        // update pointer if needed
        if (Discard)
            BaseType::Offset = New;

        return true;
    }

    template <
        typename T,
        std::enable_if_t<std::is_trivially_copyable<T>::value, bool> = true>
        bool Write(const T& Value, bool Update = false) noexcept
    {
        uint32_t Target = 0, Before = 0;

        if (!ValidateForWrite(0, sizeof(T), &Before, &Target))
            return false;

        Base::ToBytes(Value, Pointer(Target));

        if (Update)
            BaseType::Offset = Target;

        return true;
    }

    bool Write(const ByteType* Buffer, size_t Size, bool Update = false) noexcept
    {
        uint32_t Target = 0, Before = 0;

        SizeType SizeCasted = 0;
        if (!Base::IntegerTestCast<SizeType>(Size, SizeCasted))
            return false;

        if (!ValidateForWrite(0, SizeCasted, &Before, &Target))
            return false;

        if (Buffer)
            std::memcpy(Pointer(Target), Buffer, SizeCasted);

        if (Update)
            BaseType::Offset = Target;

        return true;
    }

    inline bool SanityCheck(const DataAreaRegister& State)
    {
        // Assume that alignment is power of 2
        if (State.Alignment & (State.Alignment - 1))
        {
            DASSERT(false);
            return false;
        }

        // Test alignment
        const auto TestMask = State.Alignment - 1;
        if ((State.Base & TestMask) ||
            (State.Offset & TestMask) ||
            (State.Size & TestMask))
        {
            DASSERT(false);
            return false;
        }

        // Test out of bound
        if (!(0 <= State.Offset && State.Offset <= State.Size))
        {
            DASSERT(false);
            return false;
        }

        return true;
    }
};

static_assert(
    std::is_standard_layout<VMStack>::value &&
    std::is_trivially_copyable<VMStack>::value,
    "type is not standard layout");



