//
// Created by Duzhong Chen on 2021/4/4.
//

#include <cstring>
#include "PrivateStringUtils.h"

constexpr int lencmp(uint32_t lhs, uint32_t rhs) noexcept
{
    return lhs == rhs ? 0 :
           lhs >  rhs ? 1 :
           /* else */  -1 ;
}

static int ucstrncmp(const char16_t *a, const char16_t *b, size_t l) {
    return std::memcmp(a, b, l * sizeof(char16_t ));
}

uint32_t PrivateStringUtils::ustrlen(const char16_t *str) noexcept {
    uint32_t result = 0;

    if (sizeof(wchar_t) == sizeof(char16_t))
        return wcslen(reinterpret_cast<const wchar_t *>(str));

    while (*str++)
        ++result;
    return result;
}

int PrivateStringUtils::ucstrcmp(const char16_t *a, size_t alen, const char16_t *b, size_t blen) {
    if (a == b && alen == blen)
        return 0;
    const size_t l = std::min(alen, blen);
    int cmp = ucstrncmp(a, b, l);
    return cmp ? cmp : lencmp(alen, blen);
}
