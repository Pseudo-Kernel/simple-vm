#pragma once

#include <cstdint>
#include <memory>

class Bitmap
{
public:
    using BitPosition = size_t;

    Bitmap() : BitsUnique_(), Bits_(), BitCount_()
    {
    }

    Bitmap(BitPosition BitCount, uint8_t* Bits = nullptr) :
        BitCount_(BitCount), Bits_(Bits), BitsUnique_()
    {
        DASSERT(BitCount);

        if (!Bits)
        {
            size_t Bytes = (BitCount + 8 - 1) >> 3;
            BitsUnique_ = std::make_unique<uint8_t[]>(Bytes);
            Bits_ = BitsUnique_.get();
            memset(Bits_, 0, Bytes);
        }
    }

    BitPosition Count() const
    {
        return BitCount_;
    }

    const uint8_t* Bits() const
    {
        return Bits_;
    }

    bool Get(BitPosition Index, bool& State)
    {
        if (0 <= Index && Index < BitCount_)
        {
            State = !!(Bits_[Index >> 3] & (1 << (Index & 7)));
            return true;
        }

        return false;
    }

    bool Set(BitPosition Index)
    {
        bool PrevState = false;
        return Set(Index, PrevState);
    }

    bool Set(BitPosition Index, bool& PrevState)
    {
        if (0 <= Index && Index < BitCount_)
        {
            uint8_t Bit = 1 << (Index & 7);
            uint8_t& Byte = Bits_[Index >> 3];
            PrevState = !!(Byte & Bit);
            Byte |= Bit;
            return true;
        }

        return false;
    }

    bool Clear(BitPosition Index)
    {
        bool PrevState = false;
        return Clear(Index, PrevState);
    }

    bool Clear(BitPosition Index, bool& PrevState)
    {
        if (0 <= Index && Index < BitCount_)
        {
            uint8_t Bit = 1 << (Index & 7);
            uint8_t& Byte = Bits_[Index >> 3];
            PrevState = !!(Byte & Bit);
            Byte &= ~Bit;
            return true;
        }

        return false;
    }

    bool SetRange(BitPosition Index, BitPosition Count)
    {
        BitPosition Start = Index;
        BitPosition End = Index + Count - 1;

        if (Start > End)
            return false;

        if (!(0 <= Start && End < BitCount_))
            return false;

        uint8_t* p = Bits_ + (Start >> 3);
        uint8_t* p_end = Bits_ + (End >> 3);
        size_t u = Start & 7;
        size_t v = End & 7;

        if ((Start >> 3) == (End >> 3))
        {
            // 2 ~ 6
            // -> (~((1 << 2) - 1) & ((1 << (6 + 1)) - 1)
            // same byte
            *p |= ~((1 << u) - 1) & ((1 << (v + 1)) - 1);
            return true;
        }

        if (u)
        {
            *p |= ~((1 << u) - 1); p++;
        }

        while (p < p_end)
        {
            *p = 0xff; p++;
        }

        if (v)
        {
            *p |= (1 << (v + 1)) - 1; p++;
        }
        else
        {
            *p_end = 0xff;
        }

        return true;
    }

    bool ClearRange(BitPosition Index, BitPosition Count)
    {
        BitPosition Start = Index;
        BitPosition End = Index + Count - 1;

        if (Start > End)
            return false;

        if (!(0 <= Start && End < BitCount_))
            return false;

        uint8_t* p = Bits_ + (Start >> 3);
        uint8_t* p_end = Bits_ + (End >> 3);
        size_t u = Start & 7;
        size_t v = End & 7;

        if ((Start >> 3) == (End >> 3))
        {
            // 2 ~ 6
            // -> (~((1 << 2) - 1) & ((1 << (6 + 1)) - 1)
            // same byte
            *p &= ~(~((1 << u) - 1) & ((1 << (v + 1)) - 1));
            return true;
        }

        if (u)
        {
            *p &= ((1 << u) - 1); p++;
        }

        while (p < p_end)
        {
            *p = 0; p++;
        }

        if (v)
        {
            *p &= ~((1 << (v + 1)) - 1); p++;
        }
        else
        {
            *p_end = 0;
        }

        return true;
    }

    void SetAll()
    {
        SetRange(0, BitCount_);
    }

    void ClearAll()
    {
        ClearRange(0, BitCount_);
    }

    BitPosition FindFirstClear(BitPosition Start)
    {
        // NOTE: Find the better way to do this...
        for (BitPosition i = Start; i < BitCount_; i++)
        {
            if (!(Bits_[i >> 3] & (1 << (i & 7))))
                return i;
        }

        return Position::Invalid;
    }

    BitPosition FindFirstSet(BitPosition Start)
    {
        // NOTE: Find the better way to do this...
        for (BitPosition i = Start; i < BitCount_; i++)
        {
            if (Bits_[i >> 3] & (1 << (i & 7)))
                return i;
        }

        return Position::Invalid;
    }

    BitPosition FindLastClear(BitPosition Start)
    {
        if (Start >= BitCount_)
            return Position::Invalid;

        // NOTE: Find the better way to do this...
        auto i = Start;
        while (i != Position::Invalid)
        {
            if (!(Bits_[i >> 3] & (1 << (i & 7))))
                return i;
            i--;
        }

        return Position::Invalid;
    }

    BitPosition FindLastSet(BitPosition Start)
    {
        if (Start >= BitCount_)
            return Position::Invalid;

        // NOTE: Find the better way to do this...
        auto i = Start;
        while (i != Position::Invalid)
        {
            if (Bits_[i >> 3] & (1 << (i & 7)))
                return i;
            i--;
        }

        return Position::Invalid;
    }

    struct Position
    {
        constexpr static const BitPosition Invalid = ~0;
    };

private:
    std::unique_ptr<uint8_t[]> BitsUnique_;
    uint8_t* Bits_;
    size_t BitCount_;
};
