//
// Created by Duzhong Chen on 2021/3/24.
//

#include "QArrayData.h"
#include "CommonArrayOps.h"
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <numeric>

#define Q_UNLIKELY(S) S

struct CalculateGrowingBlockSizeResult
{
    uint32_t size;
    uint32_t elementCount;
};

template <typename T> inline
typename std::enable_if<std::is_unsigned<T>::value || std::is_signed<T>::value, bool>::type
add_overflow(T v1, T v2, T *r)
{ return __builtin_add_overflow(v1, v2, r); }

template <typename T> inline
typename std::enable_if<std::is_unsigned<T>::value || std::is_signed<T>::value, bool>::type
sub_overflow(T v1, T v2, T *r)
{ return __builtin_sub_overflow(v1, v2, r); }

template <typename T> inline
typename std::enable_if<std::is_unsigned<T>::value || std::is_signed<T>::value, bool>::type
mul_overflow(T v1, T v2, T *r)
{ return __builtin_mul_overflow(v1, v2, r); }

uint32_t qCalculateBlockSize(uint32_t elementCount, uint32_t elementSize, uint32_t headerSize) noexcept
{
    assert(elementSize);

    size_t bytes;
    if (Q_UNLIKELY(mul_overflow(size_t(elementSize), size_t(elementCount), &bytes)) ||
        Q_UNLIKELY(add_overflow(bytes, size_t(headerSize), &bytes)))
        return -1;
    if (Q_UNLIKELY(uint32_t(bytes) < 0))
        return -1;

    return uint32_t(bytes);
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
    if (Q_UNLIKELY(uint32_t(morebytes) < 0)) {
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

static inline uint32_t calculateBlockSize(uint32_t &capacity, uint32_t objectSize, uint32_t headerSize, QArrayData::AllocationOption option)
{
    // Calculate the byte size
    // allocSize = objectSize * capacity + headerSize, but checked for overflow
    // plus padded to grow in size
    if (option == QArrayData::Grow) {
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
    if (Q_UNLIKELY(allocSize < 0))
        return -1;
    if (Q_UNLIKELY(add_overflow(allocSize, extra, &allocSize)))
        return -1;
    return allocSize;
}

static QArrayData *allocateData(uint32_t allocSize)
{
    QArrayData *header = static_cast<QArrayData *>(::malloc(size_t(allocSize)));
    if (header) {
        header->ref_.store(1, std::memory_order_relaxed);
        header->flags = 0;
        header->alloc = 0;
    }
    return header;
}

void *QArrayData::allocate(QArrayData **dptr, uint32_t objectSize, uint32_t capacity, QArrayData::AllocationOption option) noexcept
{
    assert(dptr);

    if (capacity == 0) {
        *dptr = nullptr;
        return nullptr;
    }

    uint32_t headerSize = sizeof(QArrayData);

    uint32_t allocSize = calculateBlockSize(capacity, objectSize, headerSize, option);
    allocSize = reserveExtraBytes(allocSize);
    if (Q_UNLIKELY(allocSize < 0)) {  // handle overflow. cannot allocate reliably
        *dptr = nullptr;
        return nullptr;
    }

    QArrayData *header = allocateData(allocSize);
    void *data = nullptr;
    if (header) {
        // find where offset should point to so that data() is aligned to alignment bytes
        data = reinterpret_cast<void*>(intptr_t(header) + headerSize);
        header->alloc = uint32_t(capacity);
    }

    *dptr = header;
    return data;
}

std::pair<QArrayData *, void *>
QArrayData::reallocateUnaligned(QArrayData *data, void *dataPointer,
                                uint32_t objectSize, uint32_t capacity, AllocationOption option) noexcept
{
    assert(!data || !data->isShared());

    const uint32_t headerSize = sizeof(QArrayData);
    uint32_t allocSize = calculateBlockSize(capacity, objectSize, headerSize, option);
    const intptr_t offset = dataPointer
                            ? reinterpret_cast<char *>(dataPointer) - reinterpret_cast<char *>(data)
                            : headerSize;
    assert(offset > 0);
    assert(offset <= allocSize); // equals when all free space is at the beginning

    allocSize = reserveExtraBytes(allocSize);
    if (Q_UNLIKELY(allocSize < 0))  // handle overflow. cannot reallocate reliably
        return std::make_pair(data, dataPointer);

    QArrayData *header = static_cast<QArrayData *>(::realloc(data, size_t(allocSize)));
    if (header) {
        header->alloc = capacity;
        dataPointer = reinterpret_cast<char *>(header) + offset;
    } else {
        dataPointer = nullptr;
    }
    return std::make_pair(static_cast<QArrayData *>(header), dataPointer);
}

void QArrayData::deallocate(QArrayData *data, uint32_t objectSize,
                            uint32_t alignment) noexcept
{
    // Alignment is a power of two
    assert(alignment >= uint32_t(alignof(QArrayData))
             && !(alignment & (alignment - 1)));

    ::free(data);
}
