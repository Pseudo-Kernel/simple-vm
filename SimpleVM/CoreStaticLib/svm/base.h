#pragma once

#include "arch.h"

namespace Base
{
    static_assert(
        sizeof(float) == sizeof(int32_t),
        "unexpected size of float");
    static_assert(
        sizeof(double) == sizeof(int64_t),
        "unexpected size of double");

    inline void Assert(bool Condition)
    {
        if (!Condition)
            __debugbreak();
    }

    /// <summary>
    /// Returns the type-punned value of type TDest.
    /// </summary>
    /// <typeparam name="TDest"></typeparam>
    /// <typeparam name="TSource"></typeparam>
    /// <typeparam name="">Not used.</typeparam>
    /// <param name="Source">Value to be converted.</param>
    /// <returns>Value of type TDest.</returns>
    template <
        typename TDest,
        typename TSource,
        typename = std::enable_if_t<
            sizeof(TDest) == sizeof(TSource) &&
            std::is_trivially_copyable<TDest>::value &&
            std::is_trivially_copyable<TSource>::value>>
    inline TDest BitCast(const TSource& Source)
    {
        alignas(alignof(TSource)) TDest Dest {};
        // this is what std::bit_cast does!
        std::memcpy(&Dest, &Source, sizeof(TDest));
        return Dest;
    }

    template <
        typename T,
        typename = std::enable_if_t<std::is_trivially_copyable<T>::value>>
    inline T FromBytes(const unsigned char* Buffer)
    {
        // make sure that source and destination have same alignment
        constexpr const auto Size = sizeof(T);
        alignas(alignof(T)) unsigned char Dest[Size]{};

        // maybe-unaligned(Buffer) to aligned(Dest) copy
        std::memcpy(Dest, Buffer, Size); // make sure that Dest is aligned

        return BitCast<T>(Dest);
    }

    template <
        typename T,
        typename = std::enable_if_t<std::is_trivially_copyable<T>::value>>
    inline void ToBytes(const T& Value, unsigned char* Buffer)
    {
        // make sure that source and destination have same alignment
        constexpr const auto Size = sizeof(T);
        alignas(alignof(T)) unsigned char Dest[Size]{};
        std::memcpy(Dest, &Value, Size); // make sure that Dest is aligned

        // aligned(Dest) to maybe-unaligned(Buffer) copy
        std::memcpy(Buffer, Dest, Size);
    }

    template <
        typename T,
        std::enable_if_t<
            std::is_integral<T>::value && sizeof(T) == sizeof(uint8_t),
            bool> = true>
    constexpr inline T Bswap(T Value)
    {
        return Value;
    }

    template <
        typename T,
        std::enable_if_t<
            std::is_integral<T>::value && sizeof(T) == sizeof(uint16_t),
            bool> = true>
    inline T Bswap(T Value)
    {
        // 16-bit swap
        std::make_unsigned_t<T> u = Value;
        return
            ((u & 0xff00) >> 0x08) |
            ((u & 0x00ff) << 0x08);
    }

    template <
        typename T,
        std::enable_if_t<
            std::is_integral<T>::value && sizeof(T) == sizeof(uint32_t),
            bool> = true>
    inline T Bswap(T Value)
    {
        // 32-bit swap
        std::make_unsigned_t<T> u = Value;
        return
            ((u & 0xff000000u) >> 0x18) |
            ((u & 0x00ff0000u) >> 0x08) |
            ((u & 0x0000ff00u) << 0x08) |
            ((u & 0x000000ffu) << 0x18);
    }

    template <
        typename T,
        std::enable_if_t<
            std::is_integral<T>::value && sizeof(T) == sizeof(uint64_t),
            bool> = true>
    inline T Bswap(T Value)
    {
        // 64-bit swap
        std::make_unsigned_t<T> u = Value;
        return
            ((u & 0xff00000000000000ull) >> 0x38) |
            ((u & 0x00ff000000000000ull) >> 0x28) |
            ((u & 0x0000ff0000000000ull) >> 0x18) |
            ((u & 0x000000ff00000000ull) >> 0x08) |
            ((u & 0x00000000ff000000ull) << 0x08) |
            ((u & 0x0000000000ff0000ull) << 0x18) |
            ((u & 0x000000000000ff00ull) << 0x28) |
            ((u & 0x00000000000000ffull) << 0x38);
    }

    enum class Endianness
    {
        Unknown = -1,
        Little,
        Big,
    };

    inline Endianness Endian()
    {
        unsigned char Bytes[sizeof(uint32_t)]{};
        ToBytes<uint32_t>(0x03020100, Bytes);

        if (Bytes[0] == 0)
        {
            return Endianness::Little;
        }
        else if (Bytes[0] == 3)
        {
            return Endianness::Big;
        }
        else
        {
            return Endianness::Unknown;
        }
    }

    template <
        typename T,
        std::enable_if_t<std::is_integral<T>::value, bool> = true>
    inline void ToBytesLe(const T& Value, unsigned char* Buffer)
    {
        std::make_unsigned_t<T> UnsignedValue = Value;

        for (int i = 0; i < sizeof(T); i++)
        {
            Buffer[i] = static_cast<unsigned char>(UnsignedValue & 0xff);
            UnsignedValue >>= 8;
        }
    }

    template <
        typename T,
        std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
    inline void ToBytesLe(const T& Value, unsigned char* Buffer)
    {
        using IntegralType = std::make_unsigned_t<
            typename Base::ToIntegralType<T>::type>;
        return ToBytesLe(Base::BitCast<IntegralType>(Value), Buffer);
    }

    template <
        typename T,
        std::enable_if_t<std::is_integral<T>::value, bool> = true>
    inline T FromBytesLe(unsigned char* Buffer)
    {
        T Value{};

        for (int i = sizeof(T) - 1; i >= 0; i--)
        {
            Value = (Value << 8) | (Buffer[i] & 0xff);
        }

        return Value;
    }

    template <
        typename T,
        std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
    inline T FromBytesLe(unsigned char* Buffer)
    {
        using IntegralType = std::make_unsigned_t<
            typename Base::ToIntegralType<T>::type>;
        auto IntegralValue = FromBytesLe<IntegralType>(Buffer);
        return BitCast<T>(IntegralValue);
    }

    template <
        typename T,
        std::enable_if_t<std::is_integral<T>::value, bool> = true>
    inline void ToBytesBe(const T& Value, unsigned char* Buffer)
    {
        std::make_unsigned_t<T> UnsignedValue = Value;

        for (int i = 0; i < sizeof(T); i++)
        {
            Buffer[i] = static_cast<unsigned char>((UnsignedValue >> (sizeof(T) - 1) * 8) & 0xff);
            UnsignedValue <<= 8;
        }
    }

    template <
        typename T,
        std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
    inline void ToBytesBe(const T& Value, unsigned char* Buffer)
    {
        using IntegralType = std::make_unsigned_t<
            typename Base::ToIntegralType<T>::type>;
        return ToBytesBe(Base::BitCast<IntegralType>(Value), Buffer);
    }

    template <
        typename T,
        std::enable_if_t<std::is_integral<T>::value, bool> = true>
    inline T FromBytesBe(unsigned char* Buffer)
    {
        T Value{};

        for (int i = 0; i < sizeof(T); i++)
        {
            Value = (Value << 8) | (Buffer[i] & 0xff);
        }

        return Value;
    }

    template <
        typename T,
        std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
    inline T FromBytesBe(unsigned char* Buffer)
    {
        using IntegralType = std::make_unsigned_t<
            typename Base::ToIntegralType<T>::type>;
        auto IntegralValue = FromBytesBe<IntegralType>(Buffer);
        return BitCast<T>(IntegralValue);
    }


    /// <summary>
    /// Returns whether the given value is power of 2.
    /// </summary>
    /// <typeparam name="T">Type of value to be tested.</typeparam>
    /// <typeparam name="">Not used.</typeparam>
    /// <param name="Value">Value to be tested.</param>
    /// <returns>True if given value is power of 2, false otherwise.</returns>
    template <
        typename T,
        typename = std::enable_if_t<
            std::is_integral<T>::value && std::is_unsigned<T>::value>>
    constexpr inline bool IsPowerOf2(T Value)
    {
        return !(Value & (Value - 1));
    }

    /// <summary>
    /// Converts the integer with sign-extension.
    /// </summary>
    /// <typeparam name="TDest">Destination type.</typeparam>
    /// <typeparam name="TSource">Source type.</typeparam>
    /// <typeparam name="">Not used.</typeparam>
    /// <param name="Value">Value to be converted.</param>
    /// <returns>Converted integer.</returns>
    template <
        typename TDest,
        typename TSource,
        typename = std::enable_if_t<
            std::is_integral<TSource>::value && std::is_integral<TDest>::value>>
    static TDest SignExtend(TSource Value)
    {
        // Same as (signed TDest)(signed TSource)Value
        auto Result =
            static_cast<std::make_signed_t<TDest>>(
                static_cast<std::make_signed_t<TSource>>(Value));

        return Result;
    }

    template <
        typename TDest,
        typename TSource,
        typename = std::enable_if_t<
            std::is_integral<TSource>::value && std::is_integral<TDest>::value>>
    static TDest ZeroExtend(TSource Value)
    {
        // Same as (unsigned TDest)(unsigned TSource)Value
        auto Result =
            static_cast<std::make_unsigned_t<TDest>>(
                static_cast<std::make_unsigned_t<TSource>>(Value));

        return Result;
    }

    template <typename T>
    struct ToIntegralType
    {
        static_assert(
            sizeof(T) == sizeof(int8_t) ||
            sizeof(T) == sizeof(int16_t) ||
            sizeof(T) == sizeof(int32_t) ||
            sizeof(T) == sizeof(int64_t),
            "size can be 1, 2, 4 or 8");

        using type = std::conditional_t<
            sizeof(T) == sizeof(int8_t),
            int8_t,
            std::conditional_t<
                sizeof(T) == sizeof(int16_t),
                int16_t,
                std::conditional_t<
                    sizeof(T) == sizeof(int32_t),
                    int32_t,
                    int64_t
                >
            >
        >;
    };

    template <typename T>
    struct ToFloatingPointType
    {
        static_assert(
            sizeof(T) == sizeof(float) ||
            sizeof(T) == sizeof(double),
            "size can be 4 or 8");

        using type = std::conditional_t<
            sizeof(T) == sizeof(float),
            float,
            double
        >;
    };

    template <
        typename TTo,
        typename TFrom,
        std::enable_if_t<
            std::is_integral<TTo>::value && 
            std::is_integral<TFrom>::value, bool> = true>
    TTo IntegerAssertCast(TFrom Value)
    {
        auto Result = static_cast<TTo>(Value);
        auto Expected = static_cast<TFrom>(Result);
        Assert(Expected == Value);
        return Result;
    }

    template <
        typename TTo,
        typename TFrom,
        std::enable_if_t<
            std::is_integral<TTo>::value && 
            std::is_integral<TFrom>::value, bool> = true>
    bool IntegerTestCast(TFrom Value, TTo& Result)
    {
        Result = static_cast<TTo>(Value);
        auto Expected = static_cast<TFrom>(Result);

        return (Expected == Result);
    }

    template <
        typename TTo,
        typename TFrom,
        std::enable_if_t<
            std::is_integral<TTo>::value && 
            std::is_integral<TFrom>::value, bool> = true>
    bool IntegerTestCast(TFrom Value)
    {
        TTo Result{};
        return IntegerTestCast(Value, Result);
    }


//	uint32_t AtomicExchange(uint32_t* Target, uint32_t Value)
//	{
//		return _InterlockedExchange(Target, Value);
//	}
//
//	uint16_t AtomicExchange(uint16_t* Target, uint16_t Value)
//	{
//		return _InterlockedExchange16(
//			reinterpret_cast<int16_t*>(Target), Value);
//	}
//
//	uint8_t AtomicExchange(uint8_t* Target, uint8_t Value)
//	{
//		return _InterlockedExchange8(reinterpret_cast<char*>(Target), Value);
//	}

    inline uint64_t UInt64x64To128(uint64_t v1, uint64_t v2, uint64_t* h)
    {
        uint32_t lo1 = static_cast<uint32_t>(v1);
        uint32_t lo2 = static_cast<uint32_t>(v2);
        uint32_t hi1 = static_cast<uint32_t>(v1 >> 32);
        uint32_t hi2 = static_cast<uint32_t>(v2 >> 32);

        uint64_t t1 = static_cast<uint64_t>(lo1) * lo2;
        uint64_t t2 = static_cast<uint64_t>(lo1) * hi2;
        uint64_t t3 = static_cast<uint64_t>(lo2) * hi1;
        uint64_t t4 = static_cast<uint64_t>(hi1) * hi2;

        constexpr const uint32_t uint32_mask = ~0;

        // 
        //     DDDDDDDD`dddddddd                    [t4]
        //              CCCCCCCC`cccccccc           [t3]
        //              BBBBBBBB`bbbbbbbb           [t2]
        //  +)                   AAAAAAAA`aaaaaaaa  [t1]
        // ----------------------------------------
        //      t4.h      t4.l     t3.l     t1.l
        //      carry2    t3.h     t2.l
        //                t2.h     t1.h
        //                carry1
        // 
        // carry1 = carry(t1.h + t2.l + t3.l)
        // carry2 = carry(t2.h + t3.h + t4.l + carry1)
        // 

        uint64_t a1 = t1 & uint32_mask; // [31:0]
        uint64_t a2 = (t1 >> 32) + (t2 & uint32_mask) + (t3 & uint32_mask); // [63:32] with carry
        uint64_t a3 = (a2 >> 32) + (t2 >> 32) + (t3 >> 32) + (t4 & uint32_mask); // [95:64] with carry
        uint64_t a4 = (a3 >> 32) + (t4 >> 32); // [127:96]

        uint64_t res_lo = (a2 << 32) | a1;
        uint64_t res_hi = (a4 << 32) | (a3 & uint32_mask);

        if (h)
            *h = res_hi;

        return res_lo;
    }

    inline uint64_t Int64x64To128(int64_t v1, int64_t v2, int64_t* h)
    {
        bool Negative = (v1 < 0) != (v2 < 0);

        uint64_t u_v1 = v1 < 0 ? -v1 : v1;
        uint64_t u_v2 = v2 < 0 ? -v2 : v2;

        uint64_t res_hi = 0;
        uint64_t res_lo = UInt64x64To128(u_v1, u_v2, &res_hi);

        if (Negative)
        {
            //
            // Same as:
            // IF !res_lo
            //   res_hi = ~res_hi + 1;  // carry
            // ELSE
            //   res_lo = ~res_lo + 1;
            //   res_hi = ~res_hi;      // no carry
            // FI
            //

            res_hi = ~res_hi + !res_lo; // update highword first
            res_lo = ~res_lo + !!res_lo;
        }

        if (h)
            *h = res_hi;

        return res_lo;
    }

    template <
        typename T,
        typename = std::enable_if_t<std::is_integral<T>::value>>
    constexpr inline bool IsInRange(T Start, T End, T Test)
    {
        return (Start <= Test && Test <= End);
    }
    
    template <
        typename T,
        typename = std::enable_if_t<std::is_integral<T>::value>>
    constexpr inline bool IsInRange2(T Start, T Count, T Test)
    {
		#pragma message("carefully check overflow for signed type because signed overflow is UB")
		//using UnsignedType = std::make_unsigned_t<T>;
		//UnsignedType UnsignedStart = Start;
		//UnsignedType UnsignedEnd = Start + Count - 1;
		//
		//if (End < Start)
		//	return false;

        return IsInRange(Start, Start + Count - 1, Test);
    }

}
