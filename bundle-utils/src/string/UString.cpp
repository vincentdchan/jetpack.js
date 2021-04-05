//
// Created by Duzhong Chen on 2021/3/23.
//

#include "UString.h"
#include "../Utils.h"
#include "UChar.h"
#include <algorithm>
#include <string>
#include "xxhash.h"

uint32_t ustrlen(const char16_t *str) noexcept
{
    uint32_t result = 0;

    if (sizeof(wchar_t) == sizeof(char16_t))
        return wcslen(reinterpret_cast<const wchar_t *>(str));

    while (*str++)
        ++result;
    return result;
}

static inline bool simdDecodeAscii(uint16_t*, const unsigned char*, const unsigned char*, const unsigned char*)
{
    return false;
}

static const char16_t utf8bom[] = { 0xef, 0xbb, 0xbf };

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

    if (!Traits::isTrusted && unlikely(b <= 0xC1)) {
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
    if (unlikely(bytesAvailable < charsNeeded - 1)) {
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
        if (unlikely(src == reinterpret_cast<const unsigned char*>(start))
            && end - src >= 3
            && unlikely(src[0] == utf8bom[0] && src[1] == utf8bom[1] && src[2] == utf8bom[2])) {
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

std::int32_t UString::toInt() const {
    std::int64_t result = 0;

    for (std::size_t i = 0; i < size(); i++) {
        char16_t ch = at(i);
        if (i == 0 && ch == u'0') {
            return -1;
        }
        if (!UChar::IsDecimalDigit(ch)) {
            return -1;
        }
        result = result * 10 + (ch - u'0');
        if (result > std::numeric_limits<std::int32_t>::max()) {
            return -1;
        }
    }

    return static_cast<int32_t>(result);
}

UString::UString(uint32_t size, bool init) noexcept {
    if (size <= 0) {
        d = DataPointer::fromRawData(&_empty, 0);
    } else {
        d = DataPointer(Data::allocate(size), size);
        d.data()[size] = '\0';
    }
}

UString::UString(const char16_t *unicode, int64_t size) {
    if (!unicode) {
        d.clear();
    } else {
        if (size < 0) {
            size = 0;
            while (unicode[size] != u'\0')
                ++size;
        }
        if (!size) {
            d = DataPointer::fromRawData(&_empty, 0);
        } else {
            d = DataPointer(Data::allocate(size), size);
            memcpy(d.data(), unicode, size * sizeof(char16_t));
            d.data()[size] = '\0';
        }
    }
}

UString::UString(char16_t ch) {
    d = DataPointer(Data::allocate(1), 1);
    d.data()[0] = ch;
    d.data()[1] = '\0';
}

void UString::resize(uint32_t size)
{
    if (size < 0)
        size = 0;

    const auto capacityAtEnd = capacity() - d.freeSpaceAtBegin();
    if (d->needsDetach() || size > capacityAtEnd)
        reallocData(size, UStringData::Grow);
    d.size = size;
    if (d->allocatedCapacity())
        d.data()[size] = 0;
}

void UString::resize(uint32_t size, char16_t fillChar)
{
    const uint32_t oldSize = length();
    resize(size);
    const uint32_t difference = length() - oldSize;
    if (difference > 0)
        std::fill_n(d.data() + oldSize, difference, fillChar);
}

struct  QContainerImplHelper
{
        enum CutResult { Null, Empty, Full, Subset };
        static constexpr CutResult mid(uint32_t originalLength, uint32_t *_position, uint32_t *_length)
        {
            uint32_t &position = *_position;
            uint32_t &length = *_length;
            if (position > originalLength) {
                position = 0;
                length = 0;
                return Null;
            }

            if (position < 0) {
                if (length < 0 || length + position >= originalLength) {
                    position = 0;
                    length = originalLength;
                    return Full;
                }
                if (length + position <= 0) {
                    position = length = 0;
                    return Null;
                }
                length += position;
                position = 0;
            } else if (size_t(length) > size_t(originalLength - position)) {
                length = originalLength - position;
            }

            if (position == 0 && length == originalLength)
                return Full;

            return length > 0 ? Subset : Empty;
        }
};

UString UString::mid(uint32_t position, uint32_t n) const {
    uint32_t p = position;
    uint32_t l = n;
    switch (QContainerImplHelper::mid(size(), &p, &l)) {
        case QContainerImplHelper::Null:
            return UString();
        case QContainerImplHelper::Empty:
            return UString(DataPointer::fromRawData(&_empty, 0));
        case QContainerImplHelper::Full:
            return *this;
        case QContainerImplHelper::Subset:
            return UString(constData() + p, l);
    }
    return UString();
}

UString &UString::append(char16_t ch)
{
    if (d->needsDetach() || !d->freeSpaceAtEnd())
        reallocGrowData(1);
    d->copyAppend(1, ch);
    d.data()[d.size] = '\0';
    return *this;
}

UString & UString::append(const char16_t *str, int64_t len) {
    if (len < 0) {
        len = ustrlen(str);
    }
    if (str && len > 0) {
        if (d->needsDetach() || len > d->freeSpaceAtEnd())
            reallocGrowData(len);
        static_assert(sizeof(char16_t) == sizeof(char16_t), "Unexpected difference in sizes");
        // the following should be safe as QChar uses char16_t as underlying data
        const char16_t *char16String = reinterpret_cast<const char16_t *>(str);
        d->copyAppend(char16String, char16String + len);
        d.data()[d.size] = '\0';
    }
    return *this;
}

UString &UString::append(const UString &str)
{
    if (!str.isNull()) {
        if (isNull()) {
            operator=(str);
        } else {
            if (d->needsDetach() || str.size() > d->freeSpaceAtEnd())
                reallocGrowData(str.size());
            d->copyAppend(str.d.data(), str.d.data() + str.d.size);
            d.data()[d.size] = '\0';
        }
    }
    return *this;
}

UString &UString::operator=(const UString &other) noexcept
{
    d = other.d;
    return *this;
}

const char16_t UString::_empty = 0;

void UString::truncate(uint32_t pos) {
    if (pos < size())
        resize(pos);
}

constexpr int lencmp(uint32_t lhs, uint32_t rhs) noexcept
{
    return lhs == rhs ? 0 :
           lhs >  rhs ? 1 :
           /* else */  -1 ;
}

static int ucstrncmp(const char16_t *a, const char16_t *b, size_t l) {
    return std::memcmp(a, b, l * sizeof(char16_t ));
}

static int ucstrcmp(const char16_t *a, size_t alen, const char16_t *b, size_t blen)
{
    if (a == b && alen == blen)
        return 0;
    const size_t l = std::min(alen, blen);
    int cmp = ucstrncmp(a, b, l);
    return cmp ? cmp : lencmp(alen, blen);
}

bool operator==(const UString &s1, const char16_t *s2) noexcept {
    auto rSize = ustrlen(s2);
    return ucstrcmp(s1.constData(), s1.length(), s2, rSize) == 0;
}

bool operator==(const UString &s1, const UString& s2) noexcept {
    return ucstrcmp(s1.constData(), s1.length(), s2.constData(), s2.length()) == 0;
}

UString UString::fromUtf8(const char *utf8, uint32_t size) {
    if (!size) {
        return UString();
    }
    UString result(size, false);
    char16_t *data = const_cast<char16_t *>(result.constData()); // we know we're not shared
    const char16_t *end = convertToUnicode(data, utf8, size);
    result.truncate(end - data);
    return result;
}

constexpr auto QChar_fromUcs4(char32_t c) noexcept
{
    struct R {
        char16_t chars[2];
        [[nodiscard]] constexpr uint32_t size() const noexcept { return chars[1] ? 2 : 1; }
        [[nodiscard]] constexpr const char16_t *begin() const noexcept { return chars; }
        [[nodiscard]] constexpr const char16_t *end() const noexcept { return begin() + size(); }
    };
    return requiresSurrogates(c) ? R{{highSurrogate(c),
                                             lowSurrogate(c)}} :
           R{{char16_t(c), u'\0'}} ;
}

static char16_t *UTF32_convertToUnicode(char16_t *out, const char* chars, uint32_t len) {
    enum { Data = 1 };
    const char *end = chars + len;

    union {
        unsigned int state_data[4];
        void *d[2];
    } g;

    union {
        uint32_t value;
        unsigned char tuple[4];
    } s;
    s.value = 0;

    int num = 0;

    while (chars < end) {
        s.tuple[num++] = *chars++;
        if (num == 4) {
            unsigned int code = s.value;
            for (char16_t c : QChar_fromUcs4(code))
                *out++ = c;
            num = 0;
        }
    }

    if (num) {
        *out++ = ReplacementCharacter;
    }

    return out;
}

UString UString::fromUtf32(const char32_t *u32, int64_t size) {
    if (size < 0) {
        size = 0;
        while (u32[size] != U'\0')
            ++size;
    }
    UString result;
    result.resize(((size * sizeof(char32_t )) + 7) >> 1); // worst case
    char16_t *end = UTF32_convertToUnicode(result.data(), reinterpret_cast<const char*>(u32), size * sizeof(char32_t));
    result.truncate(end - result.constData());
    return result;
}

UString UString::fromStdString(const std::string& str) {
    return fromUtf8(str.c_str(), str.size());
}

static constexpr inline bool isHighSurrogate(char32_t ucs4) noexcept
{
    return (ucs4 & 0xfffffc00) == 0xd800; // 0xd800 + up to 1023 (0x3ff)
}

static constexpr inline bool isLowSurrogate(char32_t ucs4) noexcept
{
    return (ucs4 & 0xfffffc00) == 0xdc00; // 0xdc00 + up to 1023 (0x3ff)
}

static constexpr inline char32_t surrogateToUcs4(char16_t high, char16_t low) noexcept
{
    // 0x010000 through 0x10ffff, provided params are actual high, low surrogates.
    // 0x010000 + ((high - 0xd800) << 10) + (low - 0xdc00), optimized:
    return (char32_t(high)<<10) + low - 0x35fdc00;
}

template <typename Traits, typename OutputPtr, typename InputPtr> inline
int toUtf8(uint16_t u, OutputPtr &dst, InputPtr &src, InputPtr end)
{
    if (!Traits::skipAsciiHandling && u < 0x80) {
        // U+0000 to U+007F (US-ASCII) - one byte
        Traits::appendByte(dst, (unsigned char)(u));
        return 0;
    } else if (u < 0x0800) {
        // U+0080 to U+07FF - two bytes
        // first of two bytes
        Traits::appendByte(dst, 0xc0 | (unsigned char)(u >> 6));
    } else {
        if (!isSurrogate(u)) {
            // U+0800 to U+FFFF (except U+D800-U+DFFF) - three bytes
            if (!Traits::allowNonCharacters && isNonCharacter(u))
                return Traits::Error;

            // first of three bytes
            Traits::appendByte(dst, 0xe0 | (unsigned char)(u >> 12));
        } else {
            // U+10000 to U+10FFFF - four bytes
            // need to get one extra codepoint
            if (Traits::availableUtf16(src, end) == 0)
                return Traits::EndOfString;

            uint16_t low = Traits::peekUtf16(src);
            if (!isHighSurrogate(u))
                return Traits::Error;
            if (!isLowSurrogate(low))
                return Traits::Error;

            Traits::advanceUtf16(src);
            uint32_t ucs4 = surrogateToUcs4(u, low);

            if (!Traits::allowNonCharacters && isNonCharacter(ucs4))
                return Traits::Error;

            // first byte
            Traits::appendByte(dst, 0xf0 | ((unsigned char)(ucs4 >> 18) & 0xf));

            // second of four bytes
            Traits::appendByte(dst, 0x80 | ((unsigned char)(ucs4 >> 12) & 0x3f));

            // for the rest of the bytes
            u = uint16_t(ucs4);
        }

        // second to last byte
        Traits::appendByte(dst, 0x80 | ((unsigned char)(u >> 6) & 0x3f));
    }

    // last byte
    Traits::appendByte(dst, 0x80 | (u & 0x3f));
    return 0;
}

static std::string convertFromUnicode(const char16_t * data, uint32_t len)
{
    // create a QByteArray with the worst case scenario size
    std::string result;
    result.resize(len * 3);

    unsigned char *dst = reinterpret_cast<unsigned char *>(const_cast<char *>(result.data()));
    const uint16_t *src = reinterpret_cast<const uint16_t *>(data);
    const uint16_t *const end = src + len;

    while (src != end) {
        const uint16_t *nextAscii = end;
        do {
            uint16_t u = *src++;
            int res = toUtf8<QUtf8BaseTraits>(u, dst, src, end);
            if (res < 0) {
                // encoding error - append '?'
                *dst++ = '?';
            }
        } while (src < nextAscii);
    }

    result.resize(dst - reinterpret_cast<unsigned char *>(const_cast<char *>(result.data())));
    return result;
}

std::string UString::toStdString() const {
    if (isNull() || isEmpty()) {
        return std::string();
    }
    return convertFromUnicode(constData(), size());
}

void UString::reallocData(uint32_t alloc, UStringData::AllocationOption option)
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

void UString::reallocGrowData(uint32_t n) {
    if (!n)  // expected to always allocate
        n = 1;

    if (d->needsDetach()) {
        DataPointer dd(DataPointer::allocateGrow(d, n, UStringData::GrowsAtEnd));
        dd->copyAppend(d.data(), d.data() + d.size);
        dd.data()[dd.size] = 0;
        d = dd;
    } else {
        d->reallocate(d.constAllocatedCapacity() + n, UStringData::Grow);
    }
}

static inline uint64_t murmurhash(const void *key, uint64_t len, uint64_t seed) noexcept
{
    const uint64_t m = 0xc6a4a7935bd1e995ULL;
    const int r = 47;

    uint64_t h = seed ^ (len * m);

    const unsigned char *data = reinterpret_cast<const unsigned char *>(key);
    const unsigned char *end = data + (len & ~7ul);

    while (data != end) {
        uint64_t k;
        memcpy(&k, data, sizeof(uint64_t));

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;

        data += 8;
    }

    len &= 7;
    if (len) {
        // handle the last few bytes of input
        size_t k = 0;
        end += len;

        while (data != end) {
            k <<= 8;
            k |= *data;
            ++data;
        }
        h ^= k;
        h *= m;
    }

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}

template <typename T> Q_ALWAYS_INLINE T qFromUnaligned(const void *src)
{
    T dest;
    const size_t size = sizeof(T);
#if __has_builtin(__builtin_memcpy)
    __builtin_memcpy
#else
    memcpy
#endif
            (&dest, src, size);
    return dest;
}

#define cROUNDS 1
#define dROUNDS 2

#define ROTL(x, b) (uint64_t)(((x) << (b)) | ((x) >> (64 - (b))))

#define SIPROUND                                                               \
  do {                                                                         \
    v0 += v1;                                                                  \
    v1 = ROTL(v1, 13);                                                         \
    v1 ^= v0;                                                                  \
    v0 = ROTL(v0, 32);                                                         \
    v2 += v3;                                                                  \
    v3 = ROTL(v3, 16);                                                         \
    v3 ^= v2;                                                                  \
    v0 += v3;                                                                  \
    v3 = ROTL(v3, 21);                                                         \
    v3 ^= v0;                                                                  \
    v2 += v1;                                                                  \
    v1 = ROTL(v1, 17);                                                         \
    v1 ^= v2;                                                                  \
    v2 = ROTL(v2, 32);                                                         \
  } while (0)


static uint64_t siphash(const uint8_t *in, uint64_t inlen, const uint64_t seed)
{
    /* "somepseudorandomlygeneratedbytes" */
    uint64_t v0 = 0x736f6d6570736575ULL;
    uint64_t v1 = 0x646f72616e646f6dULL;
    uint64_t v2 = 0x6c7967656e657261ULL;
    uint64_t v3 = 0x7465646279746573ULL;
    uint64_t b;
    uint64_t k0 = seed;
    uint64_t k1 = seed ^ inlen;
    int i;
    const uint8_t *end = in + (inlen & ~7ULL);
    const int left = inlen & 7;
    b = inlen << 56;
    v3 ^= k1;
    v2 ^= k0;
    v1 ^= k1;
    v0 ^= k0;

    for (; in != end; in += 8) {
        uint64_t m = qFromUnaligned<uint64_t>(in);
        v3 ^= m;

        for (i = 0; i < cROUNDS; ++i)
            SIPROUND;

        v0 ^= m;
    }


#if defined(Q_CC_GNU) && Q_CC_GNU >= 700
    QT_WARNING_DISABLE_GCC("-Wimplicit-fallthrough")
#endif
    switch (left) {
        case 7:
            b |= ((uint64_t)in[6]) << 48;
        case 6:
            b |= ((uint64_t)in[5]) << 40;
        case 5:
            b |= ((uint64_t)in[4]) << 32;
        case 4:
            b |= ((uint64_t)in[3]) << 24;
        case 3:
            b |= ((uint64_t)in[2]) << 16;
        case 2:
            b |= ((uint64_t)in[1]) << 8;
        case 1:
            b |= ((uint64_t)in[0]);
            break;
        case 0:
            break;
    }

    v3 ^= b;

    for (i = 0; i < cROUNDS; ++i)
        SIPROUND;

    v0 ^= b;

    v2 ^= 0xff;

    for (i = 0; i < dROUNDS; ++i)
        SIPROUND;

    b = v0 ^ v1 ^ v2 ^ v3;
    return b;
}

#if defined(_WIN32)
#define __SIZEOF_POINTER__ sizeof(uintptr_t)
#endif

size_t qHashBits(const void *p, size_t size, size_t seed) noexcept
{
    XXH64_hash_t hash = XXH3_64bits(p, size);
    return static_cast<size_t>(hash);
}

static_assert(sizeof(size_t) == sizeof(XXH64_hash_t), "should compile as 64bit program");
