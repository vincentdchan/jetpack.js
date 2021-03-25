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
    static UString fromUtf32(const char32_t* u32, int64_t size = -1);
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

    inline void clear()
    { if (!isNull()) *this = UString(); }

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
