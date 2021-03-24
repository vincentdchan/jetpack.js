//
// Created by Duzhong Chen on 2021/3/23.
//

#ifndef ROCKET_BUNDLE_USTRING_H
#define ROCKET_BUNDLE_USTRING_H

#include <memory>
#include <cinttypes>
#include <algorithm>
#include <atomic>
#include <functional>
#include <string>
#include <locale>
#include <codecvt>

#include "UStringData.h"
#include "UStringDataPointer.h"

class UString {
    typedef UStringData Data;
public:
    typedef QArrayDataPointer<UStringArrayOps> DataPointer;

    inline constexpr UString() noexcept {}
    UString(const char16_t *unicode, int64_t size = -1);
    UString(char16_t ch);
    inline UString(const UString &other) noexcept: d(other.d) {}
    inline UString(UString &&other) noexcept
    { d.swap(other.d); }

    inline uint32_t size() const { return d.size; }
    inline uint32_t count() const { return d.size; }
    inline uint32_t length() const { return d.size; }
    inline bool isNull() const noexcept { return d->isNull(); }
    inline bool isEmpty() const { return !size(); }

    inline const char16_t at(uint32_t i) const
    { assert(size_t(i) < size_t(size())); return d.data()[i]; }

    void resize(uint32_t size);
    void resize(uint32_t size, char16_t fillChar);

    [[nodiscard]] UString mid(uint32_t position, uint32_t n = -1) const;

    inline uint32_t capacity() const { return uint32_t(d->constAllocatedCapacity()); }

    inline void push_back(char16_t c) { append(c); }
    inline void push_back(const UString &s) { append(s); }
    UString &append(char16_t c);
    UString &append(const char16_t *uc, int64_t len = -1);
    UString &append(const UString &s);

    inline UString &operator+=(char16_t c) { return append(c); }

    inline UString &operator+=(const UString &s) { return append(s); }

    UString &operator=(const UString &) noexcept;

    friend bool operator==(const UString &s1, const char16_t *s2) noexcept;
    friend bool operator==(const UString &s1, const UString& s2) noexcept;
    friend bool operator!=(const UString &s1, const UString &s2) noexcept { return !(s1 == s2); }

    static UString fromUtf8(const char *utf8, uint32_t size);
    static UString fromStdString(const std::string&);

    std::string toStdString() const;

    void truncate(uint32_t size);

    inline const char16_t *data() const
    {
        return reinterpret_cast<const char16_t *>(d.data() ? d.data() : &_empty);
    }

    inline char16_t *data()
    {
        detach();
        assert(d.data());
        return reinterpret_cast<char16_t *>(d.data());
    }
    inline const char16_t *constData() const
    { return data(); }

    inline void detach()
    { if (d->needsDetach()) reallocData(d.size, UStringData::KeepSize); }

    std::int32_t toInt() const;

private:
    UString(uint32_t size, bool init) noexcept;
    explicit UString(DataPointer &&dd) : d(std::move(dd)) {}
    void reallocData(uint32_t alloc, UStringData::AllocationOption option);
    void reallocGrowData(uint32_t n);

    DataPointer d;
    static const char16_t _empty;

};

inline UString FromCodePoint(char32_t cp) {
    UString result;
    if (cp < 0x10000) {
        result.push_back(static_cast<char16_t>(cp));
    } else {
        result.push_back(static_cast<char16_t>(0xD800 + ((cp - 0x10000) >> 10)));
        result.push_back(static_cast<char16_t>(0xdc00 + ((cp - 0x10000) & 1023)));
    }
    return result;
}

inline std::string To_UTF8(const std::u32string &s) {
#ifndef _WIN32
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.to_bytes(s);
#else

    auto U32ToU16 = [](char32_t cUTF32, char16_t& h, char16_t& l) -> char32_t {
            if (cUTF32 < 0x10000)
            {
                h = 0;
                l = cUTF32;
                return cUTF32;
            }
            unsigned int t = cUTF32 - 0x10000;
            h = (((t << 12) >> 22) + 0xD800);
            l = (((t << 22) >> 22) + 0xDC00);
            char32_t ret = ((h << 16) | (l & 0x0000FFFF));
            return ret;
        };

        std::u16string u16;

        for (char32_t ch : s) {
            char16_t h = 0;
            char16_t l = 0;
            U32ToU16(ch, h, l);
            u16.push_back(h);
            u16.push_back(l);
        }

        return utils::To_UTF8(u16);
#endif
}

inline void AddU32ToUtf16(UString& target, char32_t code) {
    if (code < 0x10000) {
        target.push_back(code);
    }

    std::u32string tmp;
    tmp.push_back(code);

    auto utf8 = To_UTF8(tmp);
    auto utf16 = UString::fromUtf8(utf8.c_str(), utf8.size());

    target.append(utf16);
}

namespace UChar {

    inline bool IsDecimalDigit(char32_t cp) {
        return (cp >= 0x30 && cp <= 0x39);      // 0..9
    }

    inline bool IsLineTerminator(char32_t cp) {
        return (cp == 0x0A) || (cp == 0x0D) || (cp == 0x2028) || (cp == 0x2029);
    }

    static char32_t WHITE_SPACE[] = {0x1680, 0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005, 0x2006, 0x2007, 0x2008, 0x2009, 0x200A, 0x202F, 0x205F, 0x3000, 0xFEFF};

    inline bool IsWhiteSpace(char32_t cp) {
        if (cp >= 0x1680) {
            std::size_t count = sizeof(WHITE_SPACE) / sizeof(char32_t);
            for (std::size_t i = 0; i < count; i++) {
                if (WHITE_SPACE[i] == cp) return true;
            }
        }
        return (cp == 0x20) || (cp == 0x09) || (cp == 0x0B) || (cp == 0x0C) || (cp == 0xA0);
    }

    inline bool IsIdentifierStart(char32_t cp) {
        return (cp == 0x24) || (cp == 0x5F) ||  // $ (dollar) and _ (underscore)
               (cp >= 0x41 && cp <= 0x5A) ||         // A..Z
               (cp >= 0x61 && cp <= 0x7A) ||         // a..z
               (cp == 0x5C) ||                      // \ (backslash)
               ((cp >= 0x80)); // && Regex.NonAsciiIdentifierStart.test(Character.fromCodePoint(cp)));
    }

    inline bool IsIdentifierPart(char32_t cp) {
        return (cp == 0x24) || (cp == 0x5F) ||  // $ (dollar) and _ (underscore)
               (cp >= 0x41 && cp <= 0x5A) ||         // A..Z
               (cp >= 0x61 && cp <= 0x7A) ||         // a..z
               (cp >= 0x30 && cp <= 0x39) ||         // 0..9
               (cp == 0x5C) ||                      // \ (backslash)
               ((cp >= 0x80)); //&& Regex.NonAsciiIdentifierPart.test(Character.fromCodePoint(cp)));
    }

    inline bool IsHexDigit(char32_t cp) {
        return (cp >= 0x30 && cp <= 0x39) ||    // 0..9
               (cp >= 0x41 && cp <= 0x46) ||       // A..F
               (cp >= 0x61 && cp <= 0x66);         // a..f
    }

    inline bool IsOctalDigit(char32_t cp) {
        return (cp >= 0x30 && cp <= 0x37);      // 0..7
    }

}

inline const UString operator+(const UString &s1, const UString &s2)
{ UString t(s1); t += s2; return t; }
inline const UString operator+(const UString &s1, char16_t s2)
{ UString t(s1); t += s2; return t; }
inline const UString operator+(char16_t s1, const UString &s2)
{ UString t(s1); t += s2; return t; }

size_t qHashBits(const void *p, size_t size, size_t seed = 0) noexcept;

namespace std {

    template <>
    struct hash<UString>
    {
        std::size_t operator()(const UString& k) const
        {
            using std::size_t;
            using std::hash;
            using std::string;

            return qHashBits(k.constData(), k.size() * sizeof(char16_t));
        }
    };

}

#endif //ROCKET_BUNDLE_USTRING_H
