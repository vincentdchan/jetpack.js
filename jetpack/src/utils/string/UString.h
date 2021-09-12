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

#include "utils/Common.h"

using UString = std::u16string;
using UStringView = std::u16string_view;

#define UStr(STR) (UStringView(u"" STR, sizeof(u"" STR) / sizeof(char16_t) - 1))

std::string StringFromCodePoint(char32_t cp);
std::string StringFromUtf32(const char32_t* content, std::size_t size);

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

#endif //ROCKET_BUNDLE_USTRING_H
