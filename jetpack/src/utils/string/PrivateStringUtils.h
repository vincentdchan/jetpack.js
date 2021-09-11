//
// Created by Duzhong Chen on 2021/4/4.
//

#ifndef ROCKET_BUNDLE_PRIVATESTRINGUTILS_H
#define ROCKET_BUNDLE_PRIVATESTRINGUTILS_H

#include <string>

class UString;
class UStringView;

class PrivateStringUtils {
public:
    friend class UString;
    friend class UStringView;

    friend bool operator==(const UString &s1, const char16_t *s2) noexcept;
    friend bool operator==(const UString &s1, const UString& s2) noexcept;
    friend bool operator==(UStringView lhs, UStringView rhs) noexcept;

    struct ContainerImplHelper
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

private:
    static int ucstrcmp(const char16_t *a, size_t alen, const char16_t *b, size_t blen);
    static uint32_t ustrlen(const char16_t *str) noexcept;

};

#endif //ROCKET_BUNDLE_PRIVATESTRINGUTILS_H
