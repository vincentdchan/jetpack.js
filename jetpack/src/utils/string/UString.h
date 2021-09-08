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
#include <fmt/format.h>

#include "utils/Utils.h"

using UString = std::u16string;
using UStringView = std::u16string_view;

#define UStr(STR) (UStringView(u"" STR, sizeof(u"" STR) / sizeof(char16_t) - 1))

UString UStringFromCodePoint(char32_t cp);
UString UStringFromUtf8(const char* content, std::size_t size);
inline UString UStringFromStdString(const std::string& str) {
    return UStringFromUtf8(str.c_str(), str.size());
}
UString UStringFromUtf32(const char32_t* content, std::size_t size);
std::string UStringToUtf8(const UString& str);

//namespace std {
//
//    template <>
//    struct hash<UString>
//    {
//        std::size_t operator()(const UString& k) const
//        {
//            return UStringHashBits(k.constData(), k.size() * sizeof(char16_t));
//        }
//    };
//
//}

template<>
struct fmt::formatter<UString>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(UString const& content, FormatContext& ctx) {
        auto utf8 = UStringToUtf8(content);
        return fmt::format_to(ctx.out(), "{}", utf8);
    }
};


#endif //ROCKET_BUNDLE_USTRING_H
