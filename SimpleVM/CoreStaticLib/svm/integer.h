#pragma once

#include "base.h"
#include <cstdint>
#include <type_traits>

template <
    typename T,
    std::enable_if_t<std::is_integral<T>::value, bool> = true>
class BaseInteger // for integers
{
protected:
    using TSigned = std::make_signed_t<T>;
    using TUnsigned = std::make_unsigned_t<T>;

    using TInternalSigned = std::conditional_t<
        sizeof(T) == 1,
        int8_t,
        std::conditional_t<
        sizeof(T) == 2,
        int16_t,
        std::conditional_t<
        sizeof(T) == 4,
        int32_t,
        int64_t>>>;
    using TInternalUnsigned = std::make_unsigned_t<TInternalSigned>;


public:
    struct StateFlags
    {
        constexpr const static uint32_t Invalid = 1 << 0; // indicates that the value is invalid
        constexpr const static uint32_t Overflow = 1 << 1;
        constexpr const static uint32_t DivideByZero = 1 << 2;
    };

    BaseInteger() : 
        Value_(), 
        State_(StateFlags::Invalid)
    {
    }

    BaseInteger(const T& Value, uint32_t State) :
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
    uint32_t State_;
};

template <
    typename T,
    std::enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value, bool> = true>
    class Integer : public BaseInteger<T> // for unsigned integers
{
public:
    using type = typename T;

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

        uint32_t State = 0;
        if (vr < v1 || vr < v2)
            State |= StateFlags::Overflow;

        return BaseInteger<T>(vr, State);
    }

    Integer<T> Subtract(const Integer<T> Value) const
    {
        if (Invalid() || Value.Invalid())
            return{};

        TUnsigned v1 = Value_;
        TUnsigned v2 = -Value.Value_;
        TUnsigned vr = v1 + v2;

        uint32_t State = 0;
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

        uint32_t State = 0;
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

        uint32_t State = 0;

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

        uint32_t State = 0;

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

        TUnsigned vr = -Value_;

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


#if 0

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



#endif

