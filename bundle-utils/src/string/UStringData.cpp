//
// Created by Duzhong Chen on 2021/3/24.
//

#include "UStringData.h"
#include "Utils.h"
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include "../JetNumeric.h"

struct CalculateGrowingBlockSizeResult
{
    uint32_t size;
    uint32_t elementCount;
};

uint32_t qCalculateBlockSize(uint32_t elementCount, uint32_t elementSize, uint32_t headerSize) noexcept
{
    J_ASSERT(elementSize);

    uint32_t bytes;
    static_assert(sizeof(size_t) == 8, "");

    if (unlikely(mul_overflow(uint32_t(elementSize), uint32_t(elementCount), &bytes)) ||
        unlikely(add_overflow(bytes, uint32_t(headerSize), &bytes)))
        return -1;
    if (unlikely(uint32_t(bytes) < 0))
        return -1;

    return bytes;
}

constexpr inline uint64_t qNextPowerOfTwo(uint64_t v)
{
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    ++v;
    return v;
}

CalculateGrowingBlockSizeResult
qCalculateGrowingBlockSize(uint32_t elementCount, uint32_t elementSize, uint32_t headerSize) noexcept
{
    CalculateGrowingBlockSizeResult result = {
            uint32_t(-1), uint32_t(-1)
    };

    uint32_t bytes = qCalculateBlockSize(elementCount, elementSize, headerSize);
    if (bytes < 0)
        return result;

    size_t morebytes = static_cast<size_t>(qNextPowerOfTwo(uint64_t(bytes)));
    if (unlikely(uint32_t(morebytes) < 0)) {
        // grow by half the difference between bytes and morebytes
        // this slows the growth and avoids trying to allocate exactly
        // 2G of memory (on 32bit), something that many OSes can't deliver
        bytes += (morebytes - bytes) / 2;
    } else {
        bytes = uint32_t(morebytes);
    }

    result.elementCount = (bytes - headerSize) / elementSize;
    result.size = result.elementCount * elementSize + headerSize;
    return result;
}

static inline uint32_t calculateBlockSize(uint32_t &capacity, uint32_t objectSize, uint32_t headerSize, UStringData::AllocationOption option)
{
    // Calculate the byte size
    // allocSize = objectSize * capacity + headerSize, but checked for overflow
    // plus padded to grow in size
    if (option == UStringData::Grow) {
        auto r = qCalculateGrowingBlockSize(capacity, objectSize, headerSize);
        capacity = r.elementCount;
        return r.size;
    } else {
        return qCalculateBlockSize(capacity, objectSize, headerSize);
    }
}

static inline uint32_t reserveExtraBytes(uint32_t allocSize)
{
    // We deal with QByteArray and QString only
    constexpr uint32_t extra = std::max<uint32_t>(sizeof(char16_t), sizeof(char));
    if (unlikely(allocSize < 0))
        return -1;
    if (unlikely(add_overflow(allocSize, extra, &allocSize)))
        return -1;
    return allocSize;
}

static UStringData *allocateData(uint32_t allocSize)
{
    UStringData *header = static_cast<UStringData *>(::malloc(size_t(allocSize)));
    if (header) {
        header->ref_.store(1, std::memory_order_relaxed);
        header->flags = 0;
        header->alloc = 0;
    }
    return header;
}

void *UStringData::allocate(UStringData **dptr, uint32_t objectSize, uint32_t capacity, UStringData::AllocationOption option) noexcept
{
    J_ASSERT(dptr);

    if (capacity == 0) {
        *dptr = nullptr;
        return nullptr;
    }

    uint32_t headerSize = sizeof(UStringData);

    uint32_t allocSize = calculateBlockSize(capacity, objectSize, headerSize, option);
    allocSize = reserveExtraBytes(allocSize);
    if (unlikely(allocSize < 0)) {  // handle overflow. cannot allocate reliably
        *dptr = nullptr;
        return nullptr;
    }

    UStringData *header = allocateData(allocSize);
    void *data = nullptr;
    if (header) {
        // find where offset should point to so that data() is aligned to alignment bytes
        data = reinterpret_cast<void*>(intptr_t(header) + headerSize);
        header->alloc = uint32_t(capacity);
    }

    *dptr = header;
    return data;
}

[[nodiscard]] std::pair<UStringData *, char16_t *> UStringData::allocate(uint32_t capacity, AllocationOption option)
{
    UStringData *d;
    void *result = UStringData::allocate(&d, sizeof(char16_t ), capacity, option);
    return std::make_pair(static_cast<UStringData *>(d), static_cast<char16_t *>(result));
}

std::pair<UStringData *, char16_t*>
UStringData::reallocateUnaligned(UStringData *data, void* dataPointer,
                                 uint32_t capacity, AllocationOption option) noexcept
{
    const uint32_t objectSize = sizeof(char16_t );
    assert(!data || !data->isShared());

    const uint32_t headerSize = sizeof(UStringData);
    uint32_t allocSize = calculateBlockSize(capacity, objectSize, headerSize, option);
    const intptr_t offset = dataPointer
                            ? reinterpret_cast<char *>(dataPointer) - reinterpret_cast<char *>(data)
                            : headerSize;
    J_ASSERT(offset > 0);
    J_ASSERT(offset <= allocSize); // equals when all free space is at the beginning

    allocSize = reserveExtraBytes(allocSize);
    if (unlikely(allocSize < 0))  // handle overflow. cannot reallocate reliably
        return std::make_pair(data, reinterpret_cast<char16_t*>(dataPointer));

    UStringData *header = static_cast<UStringData *>(::realloc(data, size_t(allocSize)));
    if (header) {
        header->alloc = capacity;
        dataPointer = reinterpret_cast<char*>(header) + offset;
    } else {
        dataPointer = nullptr;
    }
    return std::make_pair(static_cast<UStringData *>(header), reinterpret_cast<char16_t *>(dataPointer));
}

void UStringData::deallocate(UStringData *data) noexcept {
    ::free(data);
}

char16_t* UStringData::dataStart(UStringData *data) noexcept
{
    void *start =  reinterpret_cast<void *>((uintptr_t(data) + sizeof(UStringData)));
    return static_cast<char16_t*>(start);
}
