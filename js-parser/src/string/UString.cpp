//
// Created by Duzhong Chen on 2021/3/23.
//

#include "UString.h"
#include <algorithm>

static inline bool simdDecodeAscii(uint16_t*, const unsigned char*, const unsigned char*, const unsigned char*)
{
    return false;
}

static const char16_t utf8bom[] = { 0xef, 0xbb, 0xbf };

#define Q_UNLIKELY(S) S

static constexpr int ReplacementCharacter = 0xfffd;
static constexpr int LastValidCodePoint = 0x10ffff;

inline bool isContinuationByte(unsigned char b)
{
    return (b & 0xc0) == 0x80;
}

static constexpr inline bool isSurrogate(char32_t ucs4) noexcept
{
    return (ucs4 - 0xd800u < 2048u);
}

static constexpr inline bool isNonCharacter(char32_t ucs4) noexcept
{
    return ucs4 >= 0xfdd0 && (ucs4 <= 0xfdef || (ucs4 & 0xfffe) == 0xfffe);
}

static constexpr inline char16_t highSurrogate(char32_t ucs4) noexcept
{
    return char16_t((ucs4>>10) + 0xd7c0);
}

static constexpr inline char16_t lowSurrogate(char32_t ucs4) noexcept
{
    return char16_t(ucs4%0x400 + 0xdc00);
}

static constexpr inline bool requiresSurrogates(char32_t ucs4) noexcept
{
    return (ucs4 >= 0x10000);
}

/// returns the number of characters consumed (including \a b) in case of success;
/// returns negative in case of error: Traits::Error or Traits::EndOfString
template <typename Traits, typename OutputPtr, typename InputPtr> inline
uint32_t fromUtf8(char16_t b, OutputPtr &dst, InputPtr &src, InputPtr end)
{
    uint32_t charsNeeded;
    uint32_t min_uc;
    uint32_t uc;

    if (!Traits::skipAsciiHandling && b < 0x80) {
        // US-ASCII
        Traits::appendUtf16(dst, b);
        return 1;
    }

    if (!Traits::isTrusted && Q_UNLIKELY(b <= 0xC1)) {
        // an UTF-8 first character must be at least 0xC0
        // however, all 0xC0 and 0xC1 first bytes can only produce overlong sequences
        return Traits::Error;
    } else if (b < 0xe0) {
        charsNeeded = 2;
        min_uc = 0x80;
        uc = b & 0x1f;
    } else if (b < 0xf0) {
        charsNeeded = 3;
        min_uc = 0x800;
        uc = b & 0x0f;
    } else if (b < 0xf5) {
        charsNeeded = 4;
        min_uc = 0x10000;
        uc = b & 0x07;
    } else {
        // the last Unicode character is U+10FFFF
        // it's encoded in UTF-8 as "\xF4\x8F\xBF\xBF"
        // therefore, a byte higher than 0xF4 is not the UTF-8 first byte
        return Traits::Error;
    }

    intptr_t bytesAvailable = Traits::availableBytes(src, end);
    if (Q_UNLIKELY(bytesAvailable < charsNeeded - 1)) {
        // it's possible that we have an error instead of just unfinished bytes
        if (bytesAvailable > 0 && !isContinuationByte(Traits::peekByte(src, 0)))
            return Traits::Error;
        if (bytesAvailable > 1 && !isContinuationByte(Traits::peekByte(src, 1)))
            return Traits::Error;
        return Traits::EndOfString;
    }

    // first continuation character
    b = Traits::peekByte(src, 0);
    if (!isContinuationByte(b))
        return Traits::Error;
    uc <<= 6;
    uc |= b & 0x3f;

    if (charsNeeded > 2) {
        // second continuation character
        b = Traits::peekByte(src, 1);
        if (!isContinuationByte(b))
            return Traits::Error;
        uc <<= 6;
        uc |= b & 0x3f;

        if (charsNeeded > 3) {
            // third continuation character
            b = Traits::peekByte(src, 2);
            if (!isContinuationByte(b))
                return Traits::Error;
            uc <<= 6;
            uc |= b & 0x3f;
        }
    }

    // we've decoded something; safety-check it
    if (!Traits::isTrusted) {
        if (uc < min_uc)
            return Traits::Error;
        if (isSurrogate(uc) || uc > LastValidCodePoint)
            return Traits::Error;
        if (!Traits::allowNonCharacters && isNonCharacter(uc))
            return Traits::Error;
    }

    // write the UTF-16 sequence
    if (!requiresSurrogates(uc)) {
        // UTF-8 decoded and no surrogates are required
        // detach if necessary
        Traits::appendUtf16(dst, uint16_t(uc));
    } else {
        // UTF-8 decoded to something that requires a surrogate pair
        Traits::appendUcs4(dst, uc);
    }

    Traits::advanceByte(src, charsNeeded - 1);
    return charsNeeded;
}


struct QUtf8BaseTraits
{
    static const bool isTrusted = false;
    static const bool allowNonCharacters = true;
    static const bool skipAsciiHandling = false;
    static const int Error = -1;
    static const int EndOfString = -2;

    static bool isValidCharacter(uint32_t u)
    { return int(u) >= 0; }

    static void appendByte(unsigned char*&ptr, unsigned char b)
    { *ptr++ = b; }

    static void appendByte(char *&ptr, char b)
    { *ptr++ = b; }

    static unsigned char peekByte(const unsigned char *ptr, uint32_t n = 0)
    { return ptr[n]; }

    static unsigned char peekByte(const char *ptr, int n = 0)
    { return ptr[n]; }

    static intptr_t availableBytes(const unsigned char *ptr, const unsigned char *end)
    { return end - ptr; }

    static intptr_t availableBytes(const char *ptr, const char *end)
    { return end - ptr; }

    static void advanceByte(const unsigned char *&ptr, uint32_t n = 1)
    { ptr += n; }

    static void advanceByte(const char *&ptr, int n = 1)
    { ptr += n; }

    static void appendUtf16(uint16_t *&ptr, uint16_t uc)
    { *ptr++ = uc; }

    static void appendUtf16(char16_t *&ptr, uint16_t uc)
    { *ptr++ = char16_t(uc); }

    static void appendUcs4(uint16_t *&ptr, uint32_t uc)
    {
        appendUtf16(ptr, highSurrogate(uc));
        appendUtf16(ptr, lowSurrogate(uc));
    }

    static void appendUcs4(char16_t *&ptr, char32_t uc)
    {
        appendUtf16(ptr, highSurrogate(uc));
        appendUtf16(ptr, lowSurrogate(uc));
    }

    static uint16_t peekUtf16(const uint16_t *ptr, uint32_t n = 0)
    { return ptr[n]; }

    static uint16_t peekUtf16(const char16_t *ptr, int n = 0)
    { return ptr[n]; }

    static intptr_t availableUtf16(const uint16_t *ptr, const uint16_t *end)
    { return end - ptr; }

    static intptr_t availableUtf16(const char16_t *ptr, const char16_t *end)
    { return end - ptr; }

    static void advanceUtf16(const uint16_t *&ptr, uint32_t n = 1)
    { ptr += n; }

    static void advanceUtf16(const char16_t *&ptr, int n = 1)
    { ptr += n; }

    // it's possible to output to UCS-4 too
    static void appendUtf16(uint32_t *&ptr, uint16_t uc)
    { *ptr++ = uc; }

    static void appendUtf16(char32_t *&ptr, uint16_t uc)
    { *ptr++ = char32_t(uc); }

    static void appendUcs4(uint32_t *&ptr, uint32_t uc)
    { *ptr++ = uc; }

    static void appendUcs4(char32_t *&ptr, uint32_t uc)
    { *ptr++ = char32_t(uc); }
};

char16_t *convertToUnicode(char16_t* buffer, const char* start, uint32_t size) noexcept
{
    uint16_t *dst = reinterpret_cast<uint16_t *>(buffer);
    const unsigned char *src = reinterpret_cast<const unsigned char*>(start);
    const unsigned char *end = src + size;

    // attempt to do a full decoding in SIMD
    const unsigned char *nextAscii = end;
    if (!simdDecodeAscii(dst, nextAscii, src, end)) {
        // at least one non-ASCII entry
        // check if we failed to decode the UTF-8 BOM; if so, skip it
        if (Q_UNLIKELY(src == reinterpret_cast<const unsigned char*>(start))
            && end - src >= 3
            && Q_UNLIKELY(src[0] == utf8bom[0] && src[1] == utf8bom[1] && src[2] == utf8bom[2])) {
            src += 3;
        }

        while (src < end) {
            nextAscii = end;
            if (simdDecodeAscii(dst, nextAscii, src, end))
                break;

            do {
                char16_t b = *src++;
                int res = fromUtf8<QUtf8BaseTraits>(b, dst, src, end);
                if (res < 0) {
                    // decoding error
                    *dst++ = ReplacementCharacter;
                }
            } while (src < nextAscii);
        }
    }

    return reinterpret_cast<char16_t *>(dst);
}

UString::UString(uint32_t size, bool init) noexcept {
    if (size <= 0) {
        d = DataPointer::fromRawData(&_empty, 0);
    } else {
        d = DataPointer(Data::allocate(size), size);
        d.data()[size] = '\0';
    }
}

const char16_t UString::_empty = 0;

void UString::truncate(uint32_t size) {

}

UString UString::fromUtf8(const char *utf8, uint32_t size) {
    if (size) {
        return UString();
    }
    UString result(size, false);
    char16_t *data = const_cast<char16_t *>(result.constData()); // we know we're not shared
    const char16_t *end = convertToUnicode(data, utf8, size);
    result.truncate(end - data);
    return result;
}

void UString::reallocData(uint32_t alloc, QArrayData::AllocationOption option)
{
    if (!alloc) {
        d = DataPointer::fromRawData(&_empty, 0);
        return;
    }

    // don't use reallocate path when reducing capacity and there's free space
    // at the beginning: might shift data pointer outside of allocated space
    const bool cannotUseReallocate = d.freeSpaceAtBegin() > 0;

    if (d->needsDetach() || cannotUseReallocate) {
        DataPointer dd(Data::allocate(alloc, option), std::min(alloc, d.size));
        if (dd.size > 0)
            ::memcpy(dd.data(), d.data(), dd.size * sizeof(char16_t));
        dd.data()[dd.size] = 0;
        d = dd;
    } else {
        d->reallocate(alloc, option);
    }
}
