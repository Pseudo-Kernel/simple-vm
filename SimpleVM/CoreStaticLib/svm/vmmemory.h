#pragma once

#include "vmbase.h"

namespace VM_NAMESPACE
{
    enum class MemoryType : uint32_t
    {
        Freed = 0,
        Data,
        Stack,
        Bytecode,

        UserDefinedRangeStart = 0x80000000,
        UserDefinedRangeEnd = 0xefffffff,

        Unspecified = 0xffffffff,
    };

    struct MemoryInfo
    {
        uint64_t Base;          //< VM Address (0x00000000`00000000 to 0xffffffff`fffff000)
        uint64_t Size;
        uint64_t MaximumSize;
        uintptr_t Tag;
        MemoryType Type;
    };

    struct MemoryRange
    {
        constexpr MemoryRange(uint64_t Base, uint64_t Size) :
            Base(Base), Size(Size) { }
        constexpr MemoryRange() :
            MemoryRange(0, 0) { }

        uint64_t Base;
        uint64_t Size;
    };

    class VMMemoryManager //: public MemoryManager
    {
    public:
        struct Options
        {
            constexpr static const uint32_t UsePreferredAddress = 0x00000001;
            constexpr static const uint32_t UsePreferredMemoryType = 0x00000002;
        };

        constexpr static const unsigned int PageShift = 12;
        constexpr static const unsigned int PageSize = 1 << PageShift;
        constexpr static const unsigned int PageMask = PageSize - 1;

        VMMemoryManager(size_t Size);
        ~VMMemoryManager();

        size_t Read(uint64_t Address, size_t Size, uint8_t* Buffer);
        size_t Write(uint64_t Address, size_t Size, uint8_t* Buffer);
        size_t Fill(uint64_t Address, size_t Size, uint8_t Value);
        bool ExecuteInMemoryContext(void(*Function)(uintptr_t), uintptr_t Param);
        bool ExecuteInMemoryContext(const std::function<void()>& Function);
        bool Allocate(uint64_t Address, size_t Size, MemoryType Type, intptr_t Tag, uint32_t Options, uint64_t& ResultAddress);
        bool Query(uint64_t Address, MemoryInfo& Info);
        uint64_t Free(uint64_t Base, size_t Size);

        uintptr_t Base() const;
        size_t Size() const;

        uintptr_t HostAddress(uint64_t GuestAddress, size_t Size = 0);

        // TODO: Add serialize()

    private:
        bool Initialize(size_t Size);
        void Cleanup();

        bool Reclaim(MemoryType SourceType, uint64_t ReclaimAddress, size_t ReclaimSize, MemoryType ReclaimType, intptr_t Tag, uint32_t ReclaimOptions, uint64_t& ResultAddress);
        int Merge(uint64_t Address, MemoryType Type, uint64_t& ResultAddress);

        static int Split(MemoryRange& SourceRange, uint64_t Address, size_t Size, MemoryRange SplitRange[2]);
        static long SEH_Pagefault(long ExceptionCode, void* ExceptionPointers, void* SEHContext);


        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value>>
        constexpr static T RoundupToBlocks(T Value)
        {
            return (Value + (PageSize - 1)) >> PageShift;
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value>>
        constexpr static T RounddownToBlocks(T Value)
        {
            return Value >> PageShift;
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value>>
        constexpr static T RoundupToBlockSize(T Value)
        {
            return RoundupToBlocks(Value) << PageShift;
        }

        template <
            typename T,
            typename = std::enable_if_t<std::is_integral<T>::value>>
        constexpr static T RounddownToBlockSize(T Value)
        {
            return RounddownToBlocks(Value) << PageShift;
        }

        std::map<uint64_t, MemoryInfo> MemoryMap_;
        Bitmap AllocationBitmap_;
        uint64_t Base_;
        uint64_t Size_;
    };
}

