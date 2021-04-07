//
// Created by Duzhong Chen on 2021/4/7.
//

#ifndef ROCKET_BUNDLE_JETNUMERIC_H
#define ROCKET_BUNDLE_JETNUMERIC_H

#if defined(_WIN32)

#include <intrin.h>
#include <float.h>
#define J_UMULH(v1, v2) __umulh(v1, v2);
#define J_SMULH(v1, v2) __mulh(v1, v2);
#pragma intrinsic(__umulh)
#pragma intrinsic(__mulh)

#else

typedef unsigned __int128 uint128_t;
typedef __int128 int128_t;

inline int64_t J_UMULH(int64_t v1, int64_t v2) {
    return uint128_t(v1) * uint128_t(v2) >> 64;
}

inline int64_t J_SMULH(int64_t v1, int64_t v2) {
    return int128_t(v1) * int128_t(v2) >> 64;
}

#endif

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

template <int> struct JetIntegerForSize;
template <>    struct JetIntegerForSize<1> { typedef uint8_t  Unsigned; typedef int8_t  Signed; };
template <>    struct JetIntegerForSize<2> { typedef uint16_t Unsigned; typedef int16_t Signed; };
template <>    struct JetIntegerForSize<4> { typedef uint32_t Unsigned; typedef int32_t Signed; };
template <>    struct JetIntegerForSize<8> { typedef uint64_t Unsigned; typedef int64_t Signed; };

template <typename T> inline
typename std::enable_if<std::is_unsigned<T>::value || std::is_signed<T>::value, bool>::type
mul_overflow(T v1, T v2, T *r)
{
    // use the next biggest type
    // Note: for 64-bit systems where __int128 isn't supported, this will cause an error.
    using LargerInt = JetIntegerForSize<sizeof(T) * 2>;
    using Larger = typename std::conditional<std::is_signed<T>::value,
            typename LargerInt::Signed, typename LargerInt::Unsigned>::type;
    Larger lr = Larger(v1) * Larger(v2);
    *r = T(lr);
    return lr > std::numeric_limits<T>::max() || lr < std::numeric_limits<T>::min();
}

template <uint64_t>
inline bool mul_overflow(uint64_t v1, uint64_t v2, uint64_t* r)
{
    *r = v1 * v2;
    return J_UMULH(v1, v2);
}

template <int64_t>
inline bool mul_overflow(int64_t v1, int64_t v2, int64_t* r)
{
    // This is slightly more complex than the unsigned case above: the sign bit
    // of 'low' must be replicated as the entire 'high', so the only valid
    // values for 'high' are 0 and -1. Use unsigned multiply since it's the same
    // as signed for the low bits and use a signed right shift to verify that
    // 'high' is nothing but sign bits that match the sign of 'low'.

    int64_t high = J_SMULH(v1, v2);
    *r = int64_t(uint64_t(v1) * uint64_t(v2));
    return (*r >> 63) != high;
}

#endif //ROCKET_BUNDLE_JETNUMERIC_H
