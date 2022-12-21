#pragma once

#include "base.h"
#include <cstdint>
#include <type_traits>

template <
    typename T,
    typename = std::enable_if_t<std::is_integral<T>::value>>
class LittleEndian
{
public:
    LittleEndian(const T& Value)
    {
        Set(Value);
    }

    LittleEndian(T&& Value)
    {
        Set(Value);
    }

    T Get() const
    {
        if (!NativeSame())
            return Base::Bswap(Value_);
        else
            return Value_;
    }

    T GetRaw() const
    {
        return Value_;
    }

    void Set(T Value)
    {
        if (!NativeSame())
            Value_ = Base::Bswap(Value);
        else
            Value_ = Value;
    }

    static bool NativeSame()
    {
        return (Base::Endian() == Base::Endianness::Little);
    }

private:
    T Value_;
};

static_assert(
    std::is_standard_layout<LittleEndian<int8_t>>::value &&
    std::is_trivially_copyable<LittleEndian<int8_t>>::value,
    "class type is not a standard layout type");

static_assert(
    std::is_standard_layout<LittleEndian<int16_t>>::value &&
    std::is_trivially_copyable<LittleEndian<int16_t>>::value,
    "class type is not a standard layout type");

static_assert(
    std::is_standard_layout<LittleEndian<int32_t>>::value &&
    std::is_trivially_copyable<LittleEndian<int32_t>>::value,
    "class type is not a standard layout type");

static_assert(
    std::is_standard_layout<LittleEndian<int64_t>>::value &&
    std::is_trivially_copyable<LittleEndian<int64_t>>::value,
    "class type is not a standard layout type");

static_assert(
    std::is_standard_layout<LittleEndian<uint8_t>>::value &&
    std::is_trivially_copyable<LittleEndian<uint8_t>>::value,
    "class type is not a standard layout type");

static_assert(
    std::is_standard_layout<LittleEndian<uint16_t>>::value &&
    std::is_trivially_copyable<LittleEndian<uint16_t>>::value,
    "class type is not a standard layout type");

static_assert(
    std::is_standard_layout<LittleEndian<uint32_t>>::value &&
    std::is_trivially_copyable<LittleEndian<uint32_t>>::value,
    "class type is not a standard layout type");

static_assert(
    std::is_standard_layout<LittleEndian<uint64_t>>::value &&
    std::is_trivially_copyable<LittleEndian<uint64_t>>::value,
    "class type is not a standard layout type");



template <
    typename T,
    typename = std::enable_if_t<std::is_integral<T>::value>>
class BigEndian
{
public:
    BigEndian(const T& Value)
    {
        Set(Value);
    }

    BigEndian(T&& Value)
    {
        Set(Value);
    }

    T Get() const
    {
        if (!NativeSame())
            return Base::Bswap(Value_);
        else
            return Value_;
    }

    T GetRaw() const
    {
        return Value_;
    }

    void Set(T Value)
    {
        if (!NativeSame())
            Value_ = Base::Bswap(Value);
        else
            Value_ = Value;
    }

    static bool NativeSame()
    {
        return (Base::Endian() == Base::Endianness::Big);
    }

private:
    T Value_;
};


static_assert(
    std::is_standard_layout<BigEndian<int8_t>>::value &&
    std::is_trivially_copyable<BigEndian<int8_t>>::value,
    "class type is not a standard layout type");

static_assert(
    std::is_standard_layout<BigEndian<int16_t>>::value &&
    std::is_trivially_copyable<BigEndian<int16_t>>::value,
    "class type is not a standard layout type");

static_assert(
    std::is_standard_layout<BigEndian<int32_t>>::value &&
    std::is_trivially_copyable<BigEndian<int32_t>>::value,
    "class type is not a standard layout type");

static_assert(
    std::is_standard_layout<BigEndian<int64_t>>::value &&
    std::is_trivially_copyable<BigEndian<int64_t>>::value,
    "class type is not a standard layout type");

static_assert(
    std::is_standard_layout<BigEndian<uint8_t>>::value &&
    std::is_trivially_copyable<BigEndian<uint8_t>>::value,
    "class type is not a standard layout type");

static_assert(
    std::is_standard_layout<BigEndian<uint16_t>>::value &&
    std::is_trivially_copyable<BigEndian<uint16_t>>::value,
    "class type is not a standard layout type");

static_assert(
    std::is_standard_layout<BigEndian<uint32_t>>::value &&
    std::is_trivially_copyable<BigEndian<uint32_t>>::value,
    "class type is not a standard layout type");

static_assert(
    std::is_standard_layout<BigEndian<uint64_t>>::value &&
    std::is_trivially_copyable<BigEndian<uint64_t>>::value,
    "class type is not a standard layout type");

