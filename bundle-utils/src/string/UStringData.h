//
// Created by Duzhong Chen on 2021/3/24.
//

#ifndef ROCKET_BUNDLE_USTRINGDATA_H
#define ROCKET_BUNDLE_USTRINGDATA_H

#include <cinttypes>
#include <utility>
#include <atomic>

#include "../JetFlags.h"

struct UStringData
{
    enum AllocationOption {
        Grow,
        KeepSize
    };

    enum GrowthPosition {
        GrowsAtEnd,
        GrowsAtBeginning
    };

    enum ArrayOption {
        ArrayOptionDefault = 0,
        CapacityReserved     = 0x1  //!< the capacity was reserved by the user, try to keep it
    };
    JET_DECLARE_FLAGS(ArrayOptions, ArrayOption)

    std::atomic<int32_t> ref_;
    uint32_t flags;
    uint32_t alloc;

    uint32_t allocatedCapacity() noexcept
    {
        return alloc;
    }

    uint32_t constAllocatedCapacity() const noexcept
    {
        return alloc;
    }

    /// Returns true if sharing took place
    bool ref() noexcept
    {
        ++ref_;
        return true;
    }

    /// Returns false if deallocation is necessary
    bool deref() noexcept
    {
        return --ref_ != 0;
    }

    bool isShared() const noexcept
    {
        return ref_.load(std::memory_order_relaxed) != 1;
    }

    // Returns true if a detach is necessary before modifying the data
    // This method is intentionally not const: if you want to know whether
    // detaching is necessary, you should be in a non-const function already
    bool needsDetach() const noexcept
    {
        return ref_.load(std::memory_order_relaxed) > 1;
    }

    uint32_t detachCapacity(uint32_t newSize) const noexcept
    {
        if (flags & CapacityReserved && newSize < constAllocatedCapacity())
            return constAllocatedCapacity();
        return newSize;
    }

    static void *allocate(UStringData **pdata, uint32_t objectSize,
                          uint32_t capacity, AllocationOption option = UStringData::KeepSize) noexcept;

    [[nodiscard]] static std::pair<UStringData *, char16_t*> allocate(uint32_t capacity, AllocationOption option = UStringData::KeepSize);

    [[nodiscard]] static std::pair<UStringData *, char16_t*> reallocateUnaligned(UStringData *data, void *dataPointer,
                                                                                 uint32_t newCapacity, AllocationOption option) noexcept;
    static void deallocate(UStringData *data) noexcept;

    static char16_t* dataStart(UStringData *data) noexcept;

};


#endif //ROCKET_BUNDLE_USTRINGDATA_H
