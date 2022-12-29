#pragma once

#include "base.h"
#include <cstdint>
#include <type_traits>

template <
    typename T,
    std::enable_if_t<std::is_integral<T>::value, bool> = true>
class BaseInteger // for integers
{
public:
    using TIntegerState = typename uint8_t;

    struct StateFlags
    {
        constexpr const static TIntegerState Invalid = 1 << 0; // indicates that the value is invalid
        constexpr const static TIntegerState Overflow = 1 << 1;
        constexpr const static TIntegerState DivideByZero = 1 << 2;
    };

    BaseInteger() : 
        Value_(), 
        State_(StateFlags::Invalid)
    {
    }

    BaseInteger(const T& Value, TIntegerState State) :
        Value_(Value),
        State_(State)
    {
    }

    BaseInteger(const T& Value) : 
        Value_(Value),
        State_()
    {
    }

    bool SafeValue(T& Value) const
    {
        if (Invalid())
            return false;

        Value = Value_;
        return true;
    }

    T Value() const
    {
        return Value_;
    }

    bool Invalid() const
    {
        return !!(State_ & StateFlags::Invalid);
    }

    auto State() const
    {
        return State_;
    }

protected:
    T Value_;
    TIntegerState State_;
};

//
// unsinged integer class.
//


template <
    typename T,
    typename = std::enable_if_t<std::is_integral<T>::value, T>>
    class Integer : public BaseInteger<T>
{
};

template <typename T>
    class Integer<
        T, 
        typename std::enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value, T>>
        : public BaseInteger<T>
{
public:
    using type = typename T;
    using TSigned = typename std::make_signed_t<T>;
    using TUnsigned = typename std::make_unsigned_t<T>;
    using TIntegerState = typename BaseInteger<T>::TIntegerState;

    Integer() :
        BaseInteger<T>()
    {
    }

    Integer(const T& Value) :
        BaseInteger<T>(Value)
    {
    }

    Integer(const BaseInteger<T>& Value) :
        BaseInteger<T>(Value)
    {
    }

    Integer<T> Add(const Integer<T> Value) const
    {
        if (Invalid() || Value.Invalid())
            return{};

        TUnsigned v1 = Value_;
        TUnsigned v2 = Value.Value_;
        TUnsigned vr = v1 + v2;

        TIntegerState State = 0;
        if (vr < v1 || vr < v2)
            State |= StateFlags::Overflow;

        return BaseInteger<T>(vr, State);
    }

    Integer<T> Subtract(const Integer<T> Value) const
    {
        if (Invalid() || Value.Invalid())
            return{};

        TUnsigned v1 = Value_;
        TUnsigned v2 = ~Value.Value_ + 1;
        TUnsigned vr = v1 + v2;

        TIntegerState State = 0;
        if (v1 < v2)
            State |= StateFlags::Overflow;

        return BaseInteger<T>(vr, State);
    }

    Integer<T> Multiply(const Integer<T> Value) const
    {
        if (Invalid() || Value.Invalid())
            return{};

        TUnsigned v1 = Value_;
        TUnsigned v2 = Value.Value_;
        TUnsigned vr = v1 * v2;

        TIntegerState State = 0;
        if (v2 && (vr / v2) != v1)
            State |= StateFlags::Overflow;

        return BaseInteger<T>(vr, State);
    }

    Integer<T> Divide(const Integer<T> Value) const
    {
        if (Invalid() || Value.Invalid())
            return{};

        TUnsigned v1 = Value_;
        TUnsigned v2 = Value.Value_;

        if (!v2)
            return BaseInteger<T>(0, StateFlags::Invalid | StateFlags::DivideByZero);

        TUnsigned vr = v1 / v2;

        return BaseInteger<T>(vr);
    }

    Integer<T> Remainder(const Integer<T> Value) const
    {
        if (Invalid() || Value.Invalid())
            return{};

        TUnsigned v1 = Value_;
        TUnsigned v2 = Value.Value_;

        if (!v2)
            return BaseInteger<T>(0, StateFlags::Invalid | StateFlags::DivideByZero);

        TUnsigned vr = v1 % v2;

        return BaseInteger<T>(vr);
    }

    Integer<T> And(const Integer<T> Value) const
    {
        if (Invalid() || Value.Invalid())
            return{};

        TUnsigned v1 = Value_;
        TUnsigned v2 = Value.Value_;
        TUnsigned vr = v1 & v2;

        return BaseInteger<T>(vr);
    }

    Integer<T> Or(const Integer<T> Value) const
    {
        if (Invalid() || Value.Invalid())
            return{};

        TUnsigned v1 = Value_;
        TUnsigned v2 = Value.Value_;
        TUnsigned vr = v1 | v2;

        return BaseInteger<T>(vr);
    }

    Integer<T> Xor(const Integer<T> Value) const
    {
        if (Invalid() || Value.Invalid())
            return{};

        TUnsigned v1 = Value_;
        TUnsigned v2 = Value.Value_;
        TUnsigned vr = v1 ^ v2;

        return BaseInteger<T>(vr);
    }

    Integer<T> ShiftLeft(const Integer<T> Value) const
    {
        if (Invalid() || Value.Invalid())
            return{};

        TUnsigned v1 = Value_;
        TUnsigned v2 = Value.Value_;
        TUnsigned vr = 0;

        TIntegerState State = 0;

        if (v2 >= (sizeof(T) << 3))
        {
            vr = 0;
            State |= StateFlags::Overflow;
        }
        else
        {
            if (v2 < 0)
                return BaseInteger<T>(0, StateFlags::Invalid);

            vr = v1 << v2;

            if (v1 != (vr >> v2))
                State |= StateFlags::Overflow;
        }

        return BaseInteger<T>(vr, State);
    }

    Integer<T> ShiftRight(const Integer<T> Value) const
    {
        if (Invalid() || Value.Invalid())
            return{};

        TUnsigned v1 = Value_;
        TUnsigned v2 = Value.Value_;
        TUnsigned vr = 0;

        TIntegerState State = 0;

        if (v2 >= (sizeof(T) << 3))
        {
            vr = 0;
        }
        else
        {
            if (v2 < 0)
                return BaseInteger<T>(0, StateFlags::Invalid);

            vr = v1 >> v2;
        }

        return BaseInteger<T>(vr, State);
    }

    Integer<T> Not() const
    {
        if (Invalid())
            return{};

        TUnsigned vr = ~Value_;

        return BaseInteger<T>(vr);
    }

    Integer<T> Negate() const
    {
        if (Invalid())
            return{};

        TUnsigned vr = ~Value_ + 1;

        return BaseInteger<T>(vr);
    }

    //
    // Operators.
    //

    Integer<T> operator+() const
    {
        return *this;
    }

    Integer<T> operator-() const
    {
        return Negate();
    }

    Integer<T> operator+(Integer<T> rhs) const
    {
        return Add(rhs);
    }

    Integer<T> operator-(const Integer<T> rhs) const
    {
        return Subtract(rhs);
    }

    Integer<T> operator*(const Integer<T> rhs) const
    {
        return Multiply(rhs);
    }

    Integer<T> operator/(const Integer<T> rhs) const
    {
        return Divide(rhs);
    }

    Integer<T> operator%(const Integer<T> rhs) const
    {
        return Remainder(rhs);
    }

    Integer<T> operator~() const
    {
        return Not();
    }

    Integer<T> operator&(const Integer<T> rhs) const
    {
        return And(rhs);
    }

    Integer<T> operator|(const Integer<T> rhs) const
    {
        return Or(rhs);
    }

    Integer<T> operator^(const Integer<T> rhs) const
    {
        return Xor(rhs);
    }

    Integer<T> operator<<(const Integer<T> rhs) const
    {
        return ShiftLeft(rhs);
    }

    Integer<T> operator>>(const Integer<T> rhs) const
    {
        return ShiftRight(rhs);
    }

    Integer<T>& operator+=(const Integer<T> rhs)
    {
        *this = Add(rhs);
        return *this;
    }

    Integer<T>& operator-=(const Integer<T> rhs)
    {
        *this = Subtract(rhs);
        return *this;
    }

    Integer<T>& operator*=(const Integer<T> rhs)
    {
        *this = Multiply(rhs);
        return *this;
    }

    Integer<T>& operator/=(const Integer<T> rhs)
    {
        *this = Divide(rhs);
        return *this;
    }

    Integer<T>& operator%=(const Integer<T> rhs)
    {
        *this = Remainder(rhs);
        return *this;
    }

    Integer<T>& operator&=(const Integer<T> rhs)
    {
        *this = And(rhs);
        return *this;
    }

    Integer<T>& operator|=(const Integer<T> rhs)
    {
        *this = Or(rhs);
        return *this;
    }

    Integer<T>& operator^=(const Integer<T> rhs)
    {
        *this = Xor(rhs);
        return *this;
    }

    Integer<T>& operator<<=(const Integer<T> rhs)
    {
        *this = ShiftLeft(rhs);
        return *this;
    }

    Integer<T>& operator>>=(const Integer<T> rhs)
    {
        *this = ShiftRight(rhs);
        return *this;
    }

    Integer<T>& operator++() // prefix
    {
        *this = Add(1);
        return *this;
    }

    Integer<T>& operator--() // prefix
    {
        *this = Subtract(1);
        return *this;
    }

    Integer<T> operator++(int dummy) // postfix
    {
        *this = Add(1);
        return *this;
    }

    Integer<T> operator--(int dummy) // postfix
    {
        *this = Subtract(1);
        return *this;
    }

    bool operator!() const = delete;
    bool operator&&(const Integer<T> rhs) const = delete;
    bool operator||(const Integer<T> rhs) const = delete;
};

static_assert(
    std::is_standard_layout<Integer<uint8_t>>::value &&
    std::is_trivially_copyable<Integer<uint8_t>>::value,
    "class type is not a standard layout type");

static_assert(
    std::is_standard_layout<Integer<uint16_t>>::value &&
    std::is_trivially_copyable<Integer<uint16_t>>::value,
    "class type is not a standard layout type");

static_assert(
    std::is_standard_layout<Integer<uint32_t>>::value &&
    std::is_trivially_copyable<Integer<uint32_t>>::value,
    "class type is not a standard layout type");

static_assert(
    std::is_standard_layout<Integer<uint64_t>>::value &&
    std::is_trivially_copyable<Integer<uint64_t>>::value,
    "class type is not a standard layout type");


//
// singed integer class.
//

template <typename T>
class Integer<
    T,
    typename std::enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value, T>>
    : public BaseInteger<T>
{
public:
    using type = typename T;
    using TSigned = typename std::make_signed_t<T>;
    using TUnsigned = typename std::make_unsigned_t<T>;
    using TIntegerState = typename BaseInteger<T>::TIntegerState;

    Integer() :
        BaseInteger<T>()
    {
    }

    Integer(const T& Value) :
        BaseInteger<T>(Value)
    {
    }

    Integer(const BaseInteger<T>& Value) :
        BaseInteger<T>(Value)
    {
    }

    Integer<T> Add(const Integer<T> Value) const
    {
        if (Invalid() || Value.Invalid())
            return{};

        constexpr const TUnsigned SignBit =
            static_cast<TUnsigned>(1) << ((sizeof(T) << 3) - 1);

        TUnsigned v1 = Value_;
        TUnsigned v2 = Value.Value_;
        TUnsigned vr = v1 + v2;

        TIntegerState State = 0;

        if (!((v1 ^ v2) & SignBit)) // have same sign
        {
            if ((vr ^ v1) & SignBit)
                State |= StateFlags::Overflow; // overflow if v2 and result have different sign
        }

        return BaseInteger<T>(vr, State);
    }

    Integer<T> Subtract(const Integer<T> Value) const
    {
        if (Invalid() || Value.Invalid())
            return{};

        constexpr const TUnsigned SignBit =
            static_cast<TUnsigned>(1) << ((sizeof(T) << 3) - 1);

        TUnsigned v1 = Value_;
        TUnsigned v2 = Value.Value_;
        TUnsigned vr = v1 + (~v2) + 1;

        TIntegerState State = 0;

        if ((v1 ^ v2) & SignBit)
        {
            if (!((vr ^ v2) & SignBit))
                State |= StateFlags::Overflow; // overflow if v2 and result have same sign
        }

        return BaseInteger<T>(vr, State);
    }

    Integer<T> Multiply(const Integer<T> Value) const
    {
        if (Invalid() || Value.Invalid())
            return{};

        constexpr const TUnsigned SignBit =
            static_cast<TUnsigned>(1) << ((sizeof(T) << 3) - 1);

        TUnsigned v1 = Value_;
        TUnsigned v2 = Value.Value_;

        TUnsigned v1_abs = (v1 & SignBit) ? ~v1 + 1 : v1;
        TUnsigned v2_abs = (v2 & SignBit) ? ~v2 + 1 : v2;

        bool ResultSigned = !!((v1 ^ v2) & SignBit);


        //
        // do the multiplication here...
        //

        constexpr const TUnsigned full_width = sizeof(T) << 3;
        constexpr const TUnsigned half_width = full_width >> 1;
        constexpr const TUnsigned full_mask = ~0;
        constexpr const TUnsigned half_mask = full_mask >> half_width;

        TUnsigned lo1 = static_cast<TUnsigned>(v1_abs & half_mask);   // half-width
        TUnsigned lo2 = static_cast<TUnsigned>(v2_abs & half_mask);   // half-width
        TUnsigned hi1 = static_cast<TUnsigned>(v1_abs >> half_width); // half-width
        TUnsigned hi2 = static_cast<TUnsigned>(v2_abs >> half_width); // half-width

        TUnsigned t1 = static_cast<TUnsigned>(lo1) * lo2; // full-width
        TUnsigned t2 = static_cast<TUnsigned>(lo1) * hi2; // full-width
        TUnsigned t3 = static_cast<TUnsigned>(lo2) * hi1; // full-width
        TUnsigned t4 = static_cast<TUnsigned>(hi1) * hi2; // full-width

        // 
        // example. 64-bit multiplication
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

        TUnsigned a1 = t1 & half_mask; // 1st half (lowest)
        TUnsigned a2 = (t1 >> half_width) + (t2 & half_mask) + (t3 & half_mask); // 2nd half with carry
        TUnsigned a3 = (a2 >> half_width) + (t2 >> half_width) + (t3 >> half_width) + (t4 & half_mask); // 3rd half with carry
        TUnsigned a4 = (a3 >> half_width) + (t4 >> half_width); // 4rd half (highest)

        TUnsigned res_lo = (a2 << half_width) | a1;
        TUnsigned res_hi = (a4 << half_width) | (a3 & half_mask);

        TIntegerState State = 0;
        if (res_hi || (res_lo & SignBit))
            State |= StateFlags::Overflow;

        TUnsigned signed_res_lo = res_lo;
        TUnsigned signed_res_hi = res_hi;

        if (ResultSigned)
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

            signed_res_hi = ~res_hi + !res_lo; // update highword first
            signed_res_lo = ~res_lo + !!res_lo;
        }

        TUnsigned vr = signed_res_lo;

        return BaseInteger<T>(vr, State);
    }

    Integer<T> Divide(const Integer<T> Value) const
    {
        if (Invalid() || Value.Invalid())
            return{};

        constexpr const TUnsigned SignBit = 
            static_cast<TUnsigned>(1) << ((sizeof(T) << 3) - 1);

        TUnsigned v1 = Value_;
        TUnsigned v2 = Value.Value_;

        if (!v2)
            return BaseInteger<T>(0, StateFlags::Invalid | StateFlags::DivideByZero);

        // example. x = -128 (80), y = -1 (FF) => x / y = 128 (overflow) => -128;
        if (v1 == SignBit && v2 == static_cast<TUnsigned>(~0))
            return BaseInteger<T>(v1, StateFlags::Overflow);

        bool ResultSigned = !!((v1 ^ v2) & SignBit);

        if (v1 & SignBit) v1 = ~v1 + 1;
        if (v2 & SignBit) v2 = ~v2 + 1;

        TUnsigned vr = v1 / v2;
        if (ResultSigned)
            vr = ~vr + 1;

        return BaseInteger<T>(vr);
    }

    Integer<T> Remainder(const Integer<T> Value) const
    {
        if (Invalid() || Value.Invalid())
            return{};

        constexpr const TUnsigned SignBit =
            static_cast<TUnsigned>(1) << ((sizeof(T) << 3) - 1);

        TUnsigned v1 = Value_;
        TUnsigned v2 = Value.Value_;

        TIntegerState State = 0;

        if (!v2)
            return BaseInteger<T>(0, StateFlags::Invalid | StateFlags::DivideByZero);

        // example. x = -128 (80), y = -1 (FF) => x / y = 128 (overflow) => -128;
        if (v1 == SignBit && v2 == static_cast<TUnsigned>(~0))
            State |= StateFlags::Overflow;

        bool ResultSigned = !!((v1 ^ v2) & SignBit);

        if (v1 & SignBit) v1 = ~v1 + 1;
        if (v2 & SignBit) v2 = ~v2 + 1;

        TUnsigned vr = v1 % v2;
        if (ResultSigned)
            vr = ~vr + 1;

        return BaseInteger<T>(vr, State);
    }

    Integer<T> And(const Integer<T> Value) const
    {
        if (Invalid() || Value.Invalid())
            return{};

        TUnsigned v1 = Value_;
        TUnsigned v2 = Value.Value_;
        TUnsigned vr = v1 & v2;

        return BaseInteger<T>(vr);
    }

    Integer<T> Or(const Integer<T> Value) const
    {
        if (Invalid() || Value.Invalid())
            return{};

        TUnsigned v1 = Value_;
        TUnsigned v2 = Value.Value_;
        TUnsigned vr = v1 | v2;

        return BaseInteger<T>(vr);
    }

    Integer<T> Xor(const Integer<T> Value) const
    {
        if (Invalid() || Value.Invalid())
            return{};

        TUnsigned v1 = Value_;
        TUnsigned v2 = Value.Value_;
        TUnsigned vr = v1 ^ v2;

        return BaseInteger<T>(vr);
    }

    Integer<T> ShiftLeft(const Integer<T> Value) const
    {
        if (Invalid() || Value.Invalid())
            return{};

        constexpr const TUnsigned SignBit =
            static_cast<TUnsigned>(1) << ((sizeof(T) << 3) - 1);

        TUnsigned v1 = Value_;
        TUnsigned v2 = Value.Value_;
        TUnsigned vr = 0;

        TIntegerState State = 0;

        if (v2 >= (sizeof(T) << 3))
        {
            vr = 0;
            State |= StateFlags::Overflow;
        }
        else
        {
            if (v2 & SignBit) // v2 < 0
                return BaseInteger<T>(0, StateFlags::Invalid);

            vr = v1 << v2;

            if (v1 != (vr >> v2))
                State |= StateFlags::Overflow;
        }

        return BaseInteger<T>(vr, State);
    }

    Integer<T> ShiftRight(const Integer<T> Value) const
    {
        if (Invalid() || Value.Invalid())
            return{};

        constexpr const TUnsigned Bitwidth = sizeof(T) << 3;
        constexpr const TUnsigned SignBit =
            static_cast<TUnsigned>(1) << ((sizeof(T) << 3) - 1);

        TUnsigned v1 = Value_;
        TUnsigned v2 = Value.Value_;
        TUnsigned vr = 0;

        TIntegerState State = 0;

        if (v2 >= (sizeof(T) << 3))
        {
            if (v1 & SignBit) // v1 < 0
            {
                vr = ~0;
            }
            else
            {
                vr = 0;
            }
        }
        else
        {
            if (v2 & SignBit) // v2 < 0
                return BaseInteger<T>(0, StateFlags::Invalid);

            vr = v1 >> v2;

            if (v1 & SignBit)
                vr |= ~(static_cast<TUnsigned>(~0) >> v2); // apply signed bits
        }

        return BaseInteger<T>(vr, State);
    }

    Integer<T> Not() const
    {
        if (Invalid())
            return{};

        TUnsigned vr = ~Value_;

        return BaseInteger<T>(vr);
    }

    Integer<T> Negate() const
    {
        if (Invalid())
            return{};

        TUnsigned v = Value_;
        TUnsigned vr = ~v + 1;

        return BaseInteger<T>(vr);
    }

    //
    // Operators.
    //

    Integer<T> operator+() const
    {
        return *this;
    }

    Integer<T> operator-() const
    {
        return Negate();
    }

    Integer<T> operator+(Integer<T> rhs) const
    {
        return Add(rhs);
    }

    Integer<T> operator-(const Integer<T> rhs) const
    {
        return Subtract(rhs);
    }

    Integer<T> operator*(const Integer<T> rhs) const
    {
        return Multiply(rhs);
    }

    Integer<T> operator/(const Integer<T> rhs) const
    {
        return Divide(rhs);
    }

    Integer<T> operator%(const Integer<T> rhs) const
    {
        return Remainder(rhs);
    }

    Integer<T> operator~() const
    {
        return Not();
    }

    Integer<T> operator&(const Integer<T> rhs) const
    {
        return And(rhs);
    }

    Integer<T> operator|(const Integer<T> rhs) const
    {
        return Or(rhs);
    }

    Integer<T> operator^(const Integer<T> rhs) const
    {
        return Xor(rhs);
    }

    Integer<T> operator<<(const Integer<T> rhs) const
    {
        return ShiftLeft(rhs);
    }

    Integer<T> operator >> (const Integer<T> rhs) const
    {
        return ShiftRight(rhs);
    }

    Integer<T>& operator+=(const Integer<T> rhs)
    {
        *this = Add(rhs);
        return *this;
    }

    Integer<T>& operator-=(const Integer<T> rhs)
    {
        *this = Subtract(rhs);
        return *this;
    }

    Integer<T>& operator*=(const Integer<T> rhs)
    {
        *this = Multiply(rhs);
        return *this;
    }

    Integer<T>& operator/=(const Integer<T> rhs)
    {
        *this = Divide(rhs);
        return *this;
    }

    Integer<T>& operator%=(const Integer<T> rhs)
    {
        *this = Remainder(rhs);
        return *this;
    }

    Integer<T>& operator&=(const Integer<T> rhs)
    {
        *this = And(rhs);
        return *this;
    }

    Integer<T>& operator|=(const Integer<T> rhs)
    {
        *this = Or(rhs);
        return *this;
    }

    Integer<T>& operator^=(const Integer<T> rhs)
    {
        *this = Xor(rhs);
        return *this;
    }

    Integer<T>& operator<<=(const Integer<T> rhs)
    {
        *this = ShiftLeft(rhs);
        return *this;
    }

    Integer<T>& operator>>=(const Integer<T> rhs)
    {
        *this = ShiftRight(rhs);
        return *this;
    }

    Integer<T>& operator++() // prefix
    {
        *this = Add(1);
        return *this;
    }

    Integer<T>& operator--() // prefix
    {
        *this = Subtract(1);
        return *this;
    }

    Integer<T> operator++(int dummy) // postfix
    {
        *this = Add(1);
        return *this;
    }

    Integer<T> operator--(int dummy) // postfix
    {
        *this = Subtract(1);
        return *this;
    }

    bool operator!() const = delete;
    bool operator&&(const Integer<T> rhs) const = delete;
    bool operator||(const Integer<T> rhs) const = delete;
};

static_assert(
    std::is_standard_layout<Integer<int8_t>>::value &&
    std::is_trivially_copyable<Integer<int8_t>>::value,
    "class type is not a standard layout type");

static_assert(
    std::is_standard_layout<Integer<int16_t>>::value &&
    std::is_trivially_copyable<Integer<int16_t>>::value,
    "class type is not a standard layout type");

static_assert(
    std::is_standard_layout<Integer<int32_t>>::value &&
    std::is_trivially_copyable<Integer<int32_t>>::value,
    "class type is not a standard layout type");

static_assert(
    std::is_standard_layout<Integer<int64_t>>::value &&
    std::is_trivially_copyable<Integer<int64_t>>::value,
    "class type is not a standard layout type");



