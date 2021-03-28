//
// Created by Duzhong Chen on 2021/3/24.
//

#include "UStringData.h"
#include "Utils.h"
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <numeric>

#if defined(_WIN32)

#include <intrin.h>
#include <float.h>
#define Q_INTRINSIC_MUL_OVERFLOW64
#define Q_UMULH(v1, v2) __umulh(v1, v2);
#define Q_SMULH(v1, v2) __mulh(v1, v2);
#pragma intrinsic(__umulh)
#pragma intrinsic(__mulh)


#endif

struct CalculateGrowingBlockSizeResult
{
    uint32_t size;
    uint32_t elementCount;
};

#if ((defined(Q_CC_INTEL) ? (Q_CC_INTEL >= 1800 && !defined(Q_OS_WIN)) : defined(Q_CC_GNU)) \
    && Q_CC_GNU >= 500) || __has_builtin(__builtin_add_overflow)

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

#else

template <typename T> inline typename std::enable_if<std::is_unsigned<T>::value, bool>::type
add_overflow(T v1, T v2, T* r)
{
    // unsigned additions are well-defined
    *r = v1 + v2;
    return v1 > T(v1 + v2);
}

template <typename T> inline typename std::enable_if<std::is_signed<T>::value, bool>::type
add_overflow(T v1, T v2, T* r)
{
    // Here's how we calculate the overflow:
    // 1) unsigned addition is well-defined, so we can always execute it
    // 2) conversion from unsigned back to signed is implementation-
    //    defined and in the implementations we use, it's a no-op.
    // 3) signed integer overflow happens if the sign of the two input operands
    //    is the same but the sign of the result is different. In other words,
    //    the sign of the result must be the same as the sign of either
    //    operand.

    using U = typename std::make_unsigned<T>::type;
    *r = T(U(v1) + U(v2));

    // If int is two's complement, assume all integer types are too.
    if (std::is_same<int32_t, int>::value) {
        // Two's complement equivalent (generates slightly shorter code):
        //  x ^ y             is negative if x and y have different signs
        //  x & y             is negative if x and y are negative
        // (x ^ z) & (y ^ z)  is negative if x and z have different signs
        //                    AND y and z have different signs
        return ((v1 ^ *r) & (v2 ^ *r)) < 0;
    }

    bool s1 = (v1 < 0);
    bool s2 = (v2 < 0);
    bool sr = (*r < 0);
    return s1 != sr && s2 != sr;
    // also: return s1 == s2 && s1 != sr;
}

template <typename T> inline typename std::enable_if<std::is_unsigned<T>::value, bool>::type
sub_overflow(T v1, T v2, T* r)
{
    // unsigned subtractions are well-defined
    *r = v1 - v2;
    return v1 < v2;
}

template <typename T> inline typename std::enable_if<std::is_signed<T>::value, bool>::type
sub_overflow(T v1, T v2, T* r)
{
    // See above for explanation. This is the same with some signs reversed.
    // We can't use add_overflow(v1, -v2, r) because it would be UB if
    // v2 == std::numeric_limits<T>::min().

    using U = typename std::make_unsigned<T>::type;
    *r = T(U(v1) - U(v2));

    if (std::is_same<int32_t, int>::value)
        return ((v1 ^ *r) & (~v2 ^ *r)) < 0;

    bool s1 = (v1 < 0);
    bool s2 = !(v2 < 0);
    bool sr = (*r < 0);
    return s1 != sr && s2 != sr;
    // also: return s1 == s2 && s1 != sr;
}

inline bool mul_overflow(uint64_t v1, uint64_t v2, uint64_t* r)
{
    *r = v1 * v2;
    return Q_UMULH(v1, v2);
}

inline bool mul_overflow(int64_t v1, int64_t v2, int64_t* r)
{
    // This is slightly more complex than the unsigned case above: the sign bit
    // of 'low' must be replicated as the entire 'high', so the only valid
    // values for 'high' are 0 and -1. Use unsigned multiply since it's the same
    // as signed for the low bits and use a signed right shift to verify that
    // 'high' is nothing but sign bits that match the sign of 'low'.

    int64_t high = Q_SMULH(v1, v2);
    *r = int64_t(uint64_t(v1) * uint64_t(v2));
    return (*r >> 63) != high;
}

#endif

uint32_t qCalculateBlockSize(uint32_t elementCount, uint32_t elementSize, uint32_t headerSize) noexcept
{
    J_ASSERT(elementSize);

    size_t bytes;
    if (unlikely(mul_overflow(size_t(elementSize), size_t(elementCount), &bytes)) ||
        unlikely(add_overflow(bytes, size_t(headerSize), &bytes)))
        return -1;
    if (unlikely(uint32_t(bytes) < 0))
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
