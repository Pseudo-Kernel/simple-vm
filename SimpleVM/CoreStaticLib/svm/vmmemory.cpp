
#include "vmmemory.h"

#include <windows.h>

namespace VM_NAMESPACE
{
    VMMemoryManager::VMMemoryManager(size_t Size) : 
        Base_(), Size_(), MemoryMap_(), AllocationBitmap_()
    {
        DASSERT(Initialize(Size));
    }

    VMMemoryManager::~VMMemoryManager()
    {
        Cleanup();
    }

    size_t VMMemoryManager::Read(uint64_t Address, size_t Size, uint8_t* Buffer)
    {
        __try
        {
            std::memcpy(Buffer, reinterpret_cast<void*>(Base_ + Address), Size);
        }
        __except (SEH_Pagefault(GetExceptionCode(), GetExceptionInformation(), this))
        {
            return 0;
        }

        return Size;
    }

    size_t VMMemoryManager::Write(uint64_t Address, size_t Size, uint8_t* Buffer)
    {
        __try
        {
            std::memcpy(reinterpret_cast<void*>(Base_ + Address), Buffer, Size);
        }
        __except (SEH_Pagefault(GetExceptionCode(), GetExceptionInformation(), this))
        {
            return 0;
        }

        return Size;
    }

    size_t VMMemoryManager::Fill(uint64_t Address, size_t Size, uint8_t Value)
    {
        __try
        {
            std::memset(reinterpret_cast<void*>(Base_ + Address), Value, Size);
        }
        __except (SEH_Pagefault(GetExceptionCode(), GetExceptionInformation(), this))
        {
            return 0;
        }

        return Size;
    }

    bool VMMemoryManager::ExecuteInMemoryContext(void (*Function)(uintptr_t), uintptr_t Param)
    {
        __try
        {
            Function(Param);
        }
        __except (SEH_Pagefault(GetExceptionCode(), GetExceptionInformation(), this))
        {
            return false;
        }

        return true;
    }

    bool VMMemoryManager::ExecuteInMemoryContext(const std::function<void()>& Function)
    {
        __try
        {
            Function();
        }
        __except (SEH_Pagefault(GetExceptionCode(), GetExceptionInformation(), this))
        {
            return false;
        }

        return true;
    }

    bool VMMemoryManager::Allocate(uint64_t Address, size_t Size, MemoryType Type, intptr_t Tag, uint32_t Options, uint64_t& ResultAddress)
    {
        return Reclaim(MemoryType::Freed, Address, Size, Type, Tag,
            Options | Options::UsePreferredMemoryType, ResultAddress);
    }

    bool VMMemoryManager::Query(uint64_t Address, MemoryInfo& Info)
    {
        // Search by start address only
        auto Iterator = std::find_if(MemoryMap_.begin(), MemoryMap_.end(),
            [Address](auto e)
            {
                return
                    e.second.Base <= Address &&
                    Address < e.second.Base + e.second.MaximumSize;
            });

        if (Iterator == MemoryMap_.end())
            return false;

        Info = Iterator->second;

        return true;
    }

    uint64_t VMMemoryManager::Free(uint64_t Base, size_t Size)
    {
        // Address must be page-aligned
        if (Base & PageMask)
            return 0;

        MemoryInfo Info;

        if (!Query(Base, Info))
            return 0;

        DASSERT(Info.Base <= Base);
        DASSERT(!(Info.Base & PageMask));

        // Cannot free memory which is already freed
        if (Info.Type == MemoryType::Freed)
            return 0;

        size_t FreeSize = Size;
        if (!FreeSize)
        {
            auto FreeSize64 = Info.MaximumSize - (Base - Info.Base);

            // Assume higher 32bits is zero
            DASSERT(!(FreeSize64 & ~0xffffffff));
            FreeSize = static_cast<size_t>(FreeSize64);
        }

        if (!FreeSize)
            return 0;

        uint64_t FreedAddress = 0;

        if (Reclaim(Info.Type, Base, FreeSize, MemoryType::Freed, 0,
            Options::UsePreferredAddress | Options::UsePreferredMemoryType,
            FreedAddress))
        {
            DASSERT(!(FreedAddress & PageMask));

            // Merge freed blocks
            uint64_t MergedAddress = 0;
            int MergedCount = Merge(FreedAddress, MemoryType::Freed, MergedAddress);

            // Free the pages (if committed)
            VirtualFree(reinterpret_cast<void*>(Base_ + FreedAddress), FreeSize, MEM_DECOMMIT);

            // Clear allocation bitmap
            uint64_t BitIndex64 = RounddownToBlocks(FreedAddress);
            size_t BitIndex = static_cast<size_t>(BitIndex64);
            DASSERT(BitIndex == BitIndex64);

            size_t PageCount = RoundupToBlocks(FreeSize);
            AllocationBitmap_.ClearRange(BitIndex, PageCount);

            return FreeSize;
        }

        return 0;
    }

    uintptr_t VMMemoryManager::Base() const
    {
        uintptr_t Result{};
        DASSERT(Base::IntegerTestCast(Base_, Result));
        return Result;
    }

    size_t VMMemoryManager::Size() const
    {
        size_t Result{};
        DASSERT(Base::IntegerTestCast(Size_, Result));
        return Result;
    }

    uintptr_t VMMemoryManager::HostAddress(uint64_t GuestAddress, size_t Size)
    {
        auto Start = GuestAddress;
        auto End = GuestAddress + Size - 1;

        DASSERT(!(GuestAddress & ~0xffffffff));

        auto ResultAddress = Base_ + GuestAddress;

        // check whether Start is in range [0, Size_)
        if (!(Start < Size_))
            return 0;

        if (Size)
        {
            // check whether [Start, End] is in range [0, Size_)
            if (!(Start <= End && End < Size_))
                return 0;
        }

        return ResultAddress;
    }


    bool VMMemoryManager::Initialize(size_t Size)
    {
        DASSERT(!Base_);
        auto Base = VirtualAlloc(nullptr, static_cast<size_t>(Size), MEM_RESERVE, PAGE_NOACCESS);

        if (!Base)
            return false;

        Base_ = reinterpret_cast<uint64_t>(Base);
        Size_ = Size;

        const uint64_t MemoryStart = 0ull;

        MemoryInfo Info;
        Info.Base = MemoryStart;
        Info.Size = Size;
        Info.MaximumSize = Size;
        Info.Tag = 0;
        Info.Type = MemoryType::Freed;

        MemoryMap_ = { { MemoryStart, Info } };

        size_t BitCount = RoundupToBlocks(Size);
        AllocationBitmap_ = Bitmap(BitCount);

        return true;
    }

    void VMMemoryManager::Cleanup()
    {
        if (Base_)
        {
            AllocationBitmap_ = {};
            MemoryMap_.clear();

            VirtualFree(reinterpret_cast<void*>(Base_), 0, MEM_RELEASE);
            Base_ = 0;
            Size_ = 0;
        }
    }

    bool VMMemoryManager::Reclaim(MemoryType SourceType, uint64_t ReclaimAddress, size_t ReclaimSize, MemoryType ReclaimType, intptr_t Tag, uint32_t ReclaimOptions, uint64_t& ResultAddress)
    {
        DASSERT(!MemoryMap_.empty());

        size_t ActualSize = RoundupToBlockSize(ReclaimSize);
        MemoryType ActualType = SourceType;

        bool IsAddressSpecified = !!(ReclaimOptions & Options::UsePreferredAddress);
        bool IsSourceTypeSpecified = !!(ReclaimOptions & Options::UsePreferredMemoryType);

        uint64_t Start = 0;
        uint64_t End = 0;

        if (!ActualSize)
            return false;

        auto Iterator = MemoryMap_.end();

        if (IsAddressSpecified)
        {
            Start = ReclaimAddress;
            End = ReclaimAddress + ActualSize - 1;

            // Address must be page-aligned
            if (Start & PageMask)
                return false;

            // Search by start address only
            Iterator = std::find_if(MemoryMap_.begin(), MemoryMap_.end(),
                [Start](auto e)
                {
                    return
                        e.second.Base <= Start &&
                        Start < e.second.Base + e.second.MaximumSize;
                });

            if (Iterator == MemoryMap_.end())
                return false;

            // Check out of range
            if (End > Iterator->second.Base + Iterator->second.MaximumSize - 1)
                return false;

            // Check memory type if specified
            if (IsSourceTypeSpecified)
            {
                if (ActualType != Iterator->second.Type)
                    return false;
            }
        }
        else
        {
            // Find block
            Iterator = std::find_if(MemoryMap_.begin(), MemoryMap_.end(),
                [IsSourceTypeSpecified, ActualType, ActualSize](auto e)
                {
                    if (ActualSize <= e.second.MaximumSize)
                    {
                        return IsSourceTypeSpecified ?
                            (ActualType == e.second.Type) : true;
                    }

                    return false;
                });

            if (Iterator == MemoryMap_.end())
                return false;

            Start = Iterator->second.Base;
            End = Iterator->second.Base + ActualSize - 1;
            ActualType = Iterator->second.Type;

            DASSERT(Start <= End);
            DASSERT(!(Start & PageMask));
        }

        if (ActualType == ReclaimType)
            return false;

        // sanity check
        DASSERT(Iterator->first == Iterator->second.Base);

        auto OriginalInfo = Iterator->second;

        if (OriginalInfo.Type != ActualType)
            return false;

        MemoryRange SourceRange = MemoryRange(OriginalInfo.Base, OriginalInfo.Size);
        MemoryRange SplitRange[2];

        int SplitCount = Split(SourceRange, Start, ActualSize, SplitRange);

        switch (SplitCount)
        {
        case 1: // exactly fit
        {
            Iterator->second.Type = ReclaimType;
            Iterator->second.Tag = Tag;
            Iterator->second.Size = ReclaimSize;

            break;
        }
        case 2: // first/last fit
        {
            MemoryInfo NewInfo[2]{};

            NewInfo[0].Base = SourceRange.Base;
            NewInfo[0].MaximumSize = SourceRange.Size;
            NewInfo[0].Size = ReclaimSize;
            NewInfo[0].Type = ReclaimType;
            NewInfo[0].Tag = Tag;

            NewInfo[1].Base = SplitRange[0].Base;
            NewInfo[1].MaximumSize = SplitRange[0].Size;
            NewInfo[1].Size = SplitRange[0].Size;
            NewInfo[1].Type = OriginalInfo.Type;
            NewInfo[1].Tag = OriginalInfo.Tag;

            //if (OriginalInfo.Base != SourceRange.Base)
            //    std::swap(Info2, Info3);

            MemoryMap_.erase(OriginalInfo.Base);

            for (auto& it : NewInfo)
            {
                auto Result = MemoryMap_.try_emplace(it.Base, it);
                DASSERT(Result.second);
            }

            break;
        }
        case 3: // mid fit
        {
            MemoryInfo NewInfo[3]{};

            NewInfo[0].Base = SourceRange.Base;
            NewInfo[0].MaximumSize = SourceRange.Size;
            NewInfo[0].Size = ReclaimSize;
            NewInfo[0].Type = ReclaimType;
            NewInfo[0].Tag = Tag;

            NewInfo[1].Base = SplitRange[0].Base;
            NewInfo[1].MaximumSize = SplitRange[0].Size;
            NewInfo[1].Size = SplitRange[0].Size;
            NewInfo[1].Type = OriginalInfo.Type;
            NewInfo[1].Tag = OriginalInfo.Tag;

            NewInfo[2].Base = SplitRange[1].Base;
            NewInfo[2].MaximumSize = SplitRange[1].Size;
            NewInfo[2].Size = SplitRange[1].Size;
            NewInfo[2].Type = OriginalInfo.Type;
            NewInfo[2].Tag = OriginalInfo.Tag;

            MemoryMap_.erase(OriginalInfo.Base);

            for (auto& it : NewInfo)
            {
                auto Result = MemoryMap_.try_emplace(it.Base, it);
                DASSERT(Result.second);
            }

            break;
        }
        default:
            return false;
        }

        ResultAddress = SourceRange.Base;

        return true;
    }

    int VMMemoryManager::Merge(uint64_t Address, MemoryType Type, uint64_t& ResultAddress)
    {
        DASSERT(!MemoryMap_.empty());

        uint64_t TargetAddress = Address;
        int MergedCount = 0;

        while (true)
        {
            auto Iterator = std::find_if(MemoryMap_.begin(), MemoryMap_.end(),
                [TargetAddress](auto e)
                {
                    return
                        e.second.Base <= TargetAddress &&
                        TargetAddress < e.second.Base + e.second.MaximumSize;
                });

            if (Iterator == MemoryMap_.end())
                break;

            if (Iterator->second.Type != Type)
                break;

            auto IteratorPrev = Iterator;
            --IteratorPrev;
            if (IteratorPrev != MemoryMap_.end())
            {
                uint64_t ExpectedBase =
                    IteratorPrev->second.Base + IteratorPrev->second.MaximumSize;
                if (IteratorPrev->second.Type == Iterator->second.Type &&
                    ExpectedBase == Iterator->second.Base)
                {
                    // Merge!
                    TargetAddress = IteratorPrev->second.Base;
                    IteratorPrev->second.MaximumSize += Iterator->second.MaximumSize;
                    IteratorPrev->second.Size += Iterator->second.MaximumSize;
                    MemoryMap_.erase(Iterator); // Delete element by iterator
                    MergedCount++;
                    continue;
                }
            }

            auto IteratorNext = Iterator;
            ++IteratorNext;
            if (IteratorNext != MemoryMap_.end())
            {
                uint64_t ExpectedBase =
                    Iterator->second.Base + Iterator->second.MaximumSize;
                if (Iterator->second.Type == IteratorNext->second.Type &&
                    ExpectedBase == IteratorNext->second.Base)
                {
                    // Merge!
                    TargetAddress = Iterator->second.Base;
                    Iterator->second.MaximumSize += IteratorNext->second.MaximumSize;
                    Iterator->second.Size += IteratorNext->second.MaximumSize;
                    MemoryMap_.erase(IteratorNext); // Delete element by iterator
                    MergedCount++;
                    continue;
                }
            }

            break;
        }

        ResultAddress = TargetAddress;

        return MergedCount;
    }
    
    int VMMemoryManager::Split(MemoryRange& SourceRange, uint64_t Address, size_t Size, MemoryRange SplitRange[2])
    {
        uint64_t Start = SourceRange.Base;
        uint64_t End = SourceRange.Base + SourceRange.Size - 1;
        DASSERT(Start <= End);

        uint64_t TargetStart = Address;
        uint64_t TargetEnd = Address + Size - 1;
        DASSERT(TargetStart <= TargetEnd);

        if (!(Start <= TargetStart && TargetEnd <= End))
            return 0;

        if (Start == TargetStart && End == TargetEnd)
        {
            // exactly fit
            // [Start, End] => [TargetStart, TargetEnd]
            return 1;
        }

        SourceRange.Base = TargetStart;
        SourceRange.Size = Size;

        // first fit
        if (Start == TargetStart)
        {
            // [Start, End] => [TargetStart, TargetEnd+1)*, [TargetEnd+1, End)
            SplitRange[0].Base = TargetEnd + 1;
            SplitRange[0].Size = End - TargetEnd;

            return 2;
        }
        // last fit
        else if (End == TargetEnd)
        {
            // [Start, End] => [Start, TargetStart), [TargetStart, TargetEnd)*
            SplitRange[0].Base = Start;
            SplitRange[0].Size = TargetStart - Start;

            return 2;
        }
        // mid fit
        else
        {
            // [Start, End]
            // => [Start, TargetStart), [TargetStart, TargetEnd+1)*, [TargetEnd+1, End)
            SplitRange[0].Base = Start;
            SplitRange[0].Size = TargetStart - Start;
            SplitRange[1].Base = TargetEnd + 1;
            SplitRange[1].Size = End - TargetEnd;

            return 3;
        }
    }

    long VMMemoryManager::SEH_Pagefault(long Code, void *ExceptionInfo, void* SEHContext)
    {
        auto Pointers = reinterpret_cast<EXCEPTION_POINTERS*>(ExceptionInfo);
        if (Pointers->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
        {
            auto Mode = Pointers->ExceptionRecord->ExceptionInformation[0];
            uint64_t TargetAddress = Pointers->ExceptionRecord->ExceptionInformation[1];

            // Access mode: 0=read, 1=write, 8=DEP violation
            if (Mode == 1) // Write attempt?
            {
                auto This = reinterpret_cast<VMMemoryManager*>(SEHContext);
                if (Base::IsInRange2(This->Base_, This->Size_, TargetAddress))
                {
                    uint64_t NewAddress = RounddownToBlockSize(TargetAddress);

                    // Request 1 page
                    auto p = VirtualAlloc(reinterpret_cast<void*>(NewAddress),
                        PageSize, MEM_COMMIT, PAGE_READWRITE);
                    if (p)
                    {
                        uint64_t BitIndex64 = RounddownToBlocks(NewAddress - This->Base_);
                        DASSERT(!(BitIndex64 & ~0xffffffff));

                        size_t BitIndex = static_cast<size_t>(BitIndex64);
                        uint8_t Flag = 1 << (BitIndex & 7);

                        bool PrevState = false;
                        DASSERT(This->AllocationBitmap_.Set(BitIndex, PrevState));

                        // Should we check previous state?
                        // DASSERT(!PrevState);

                        // OK, continue execution
                        return EXCEPTION_CONTINUE_EXECUTION;
                    }
                }
            }
        }

        return EXCEPTION_EXECUTE_HANDLER;
    }
}
