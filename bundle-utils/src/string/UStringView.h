//
// Created by Duzhong Chen on 2021/4/4.
//

#ifndef ROCKET_BUNDLE_USTRINGVIEW_H
#define ROCKET_BUNDLE_USTRINGVIEW_H

#include "../Utils.h"
#include <string>

class UString;

class UStringView {
private:
    template <typename T>
    using if_compatible_ustring_like = typename std::enable_if<std::is_same<T, UString>::value, bool>::type;

public:
    typedef char16_t storage_type;

    constexpr UStringView() noexcept
            : m_size(0), m_data(nullptr) {}
    constexpr UStringView(std::nullptr_t) noexcept
    : UStringView() {}

    constexpr UStringView(const char16_t *str, uint32_t len)
            : m_size(len),
              m_data(str) {
      J_ASSERT(len >= 0);
      J_ASSERT(str || !len);
    }

    constexpr UStringView(const char16_t *f, const char16_t*l)
            : UStringView(f, l - f) {}

    template <typename String, if_compatible_ustring_like<String> = true>
    UStringView(const String &str) noexcept
            : UStringView(str.isNull() ? nullptr : str.data(), uint32_t (str.size())) {}

    [[nodiscard]] constexpr uint32_t size() const noexcept { return m_size; }
    [[nodiscard]] const char* data() const noexcept { return reinterpret_cast<const char*>(m_data); }
    [[nodiscard]] const char* constData() const noexcept { return data(); }
    [[nodiscard]] constexpr const storage_type *utf16() const noexcept { return m_data; }

    [[nodiscard]] constexpr char16_t operator[](uint32_t n) const {
        J_ASSERT(n >= 0);
        J_ASSERT(n < size());
        return m_data[n];
    }

    [[nodiscard]] constexpr char16_t at(uint32_t n) const noexcept { return (*this)[n]; }

    [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }

    [[nodiscard]] constexpr bool isNull() const noexcept { return !m_data; }
    [[nodiscard]] constexpr bool isEmpty() const noexcept { return empty(); }
    [[nodiscard]] constexpr int length() const /* not nothrow! */ {
        J_ASSERT(int(size()) == size());
        return int(size());
    }

    friend bool operator==(UStringView lhs, UStringView rhs) noexcept;

    [[nodiscard]] UStringView mid(uint32_t pos, uint32_t n = -1) const noexcept;

private:
    uint32_t           m_size;
    const storage_type *m_data;

};

#define UNICODE_LITERAL(str) u"" str
#define UStr(content) \
    UStringView(UNICODE_LITERAL(content), sizeof(UNICODE_LITERAL(content)) / 2 - 1)

bool operator==(UStringView lhs, UStringView rhs) noexcept;

static_assert(sizeof(std::string::size_type) == sizeof(std::size_t), "32 bit?");
static_assert(std::is_unsigned<std::string::size_type>(), "unsigned?");

#endif //ROCKET_BUNDLE_USTRINGVIEW_H
